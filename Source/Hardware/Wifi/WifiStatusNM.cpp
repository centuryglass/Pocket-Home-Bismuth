#ifdef LINUX
#include <map>
#include <nm-remote-connection.h>
#include <nm-utils.h>
#include "JuceHeader.h"
#include "WifiStatusNM.h"

#define LIBNM_ITERATION_PERIOD 100 // milliseconds

WifiStatusNM::WifiStatusNM() : connectedAP(WifiAccessPoint())
{
    if (!connectToNetworkManager())
    {
        DBG("WifiStatusNM::" << __func__
                << ": failed to connect to nmclient over dbus");
        enabled = false;
        return;
    }
    context = g_main_context_default();

    g_signal_connect_swapped(nmClient,
            "notify::" NM_CLIENT_WIRELESS_ENABLED,
            G_CALLBACK(handle_wireless_enabled), this);

    g_signal_connect_swapped(nmDevice, "notify::" NM_DEVICE_STATE,
            G_CALLBACK(handle_wireless_connected), this);

    g_signal_connect_swapped(NM_DEVICE_WIFI(nmDevice),
            "notify::" NM_DEVICE_WIFI_ACTIVE_ACCESS_POINT,
            G_CALLBACK(handle_active_access_point), this);

    g_signal_connect_swapped(NM_DEVICE_WIFI(nmDevice), "access-point-added",
            G_CALLBACK(handle_changed_access_points), this);

    g_signal_connect_swapped(NM_DEVICE_WIFI(nmDevice), "access-point-removed",
            G_CALLBACK(handle_changed_access_points), this);

    enabled = nm_client_wireless_get_enabled(nmClient);
    if (enabled)
    {
        connected = nm_device_get_state(nmDevice) == NM_DEVICE_STATE_ACTIVATED;
        if (connected)
        {
            connectedAP = getNMConnectedAP(NM_DEVICE_WIFI(nmDevice));
        }
        startThread();
    }
}

/**
 * @return true iff wifi is currently turned on.
 */
bool WifiStatusNM::isWifiDeviceEnabled()
{
    const ScopedLock lock(wifiLock);
    if (nmDevice != nullptr)
    {
        switch (nm_device_get_state(nmDevice))
        {
            case NM_DEVICE_STATE_UNMANAGED:
            case NM_DEVICE_STATE_DISCONNECTED:
            case NM_DEVICE_STATE_PREPARE:
            case NM_DEVICE_STATE_CONFIG:
            case NM_DEVICE_STATE_NEED_AUTH:
            case NM_DEVICE_STATE_IP_CONFIG:
            case NM_DEVICE_STATE_IP_CHECK:
            case NM_DEVICE_STATE_SECONDARIES:
            case NM_DEVICE_STATE_ACTIVATED:
            case NM_DEVICE_STATE_DEACTIVATING:
            case NM_DEVICE_STATE_FAILED:
                return true;
        }
    }
    return false;
}

/**
 */
bool WifiStatusNM::isWifiConnecting()
{
    const ScopedLock lock(wifiLock);
    if (nmDevice != nullptr)
    {
        switch (nm_device_get_state(nmDevice))
        {
            case NM_DEVICE_STATE_PREPARE:
            case NM_DEVICE_STATE_CONFIG:
            case NM_DEVICE_STATE_NEED_AUTH:
            case NM_DEVICE_STATE_IP_CONFIG:
            case NM_DEVICE_STATE_IP_CHECK:
            case NM_DEVICE_STATE_SECONDARIES:
                return true;
        }
    }
    return false;
}

/**
 */
bool WifiStatusNM::isWifiConnected()
{
    const ScopedLock lock(wifiLock);
    if (nmDevice != nullptr)
    {
        if (nm_device_get_state(nmDevice) == NM_DEVICE_STATE_ACTIVATED)
        {
            return true;
        }
    }
    return false;
}

/**
 */
WifiAccessPoint WifiStatusNM::getConnectedAP()
{
    const ScopedLock lock(wifiLock);

    if (nmDevice != nullptr)
    {

    }
    NMDeviceWifi* wifiDevice = NM_DEVICE_WIFI(nmDevice);
    return getNMConnectedAP(wifiDevice);
}

/**
 */
WifiAccessPoint WifiStatusNM::getConnectingAP() {
	//NMActiveConnection *conn =
        //        nm_client_get_activating_connection(nmClient);
  }

/**
 */
Array<WifiAccessPoint> WifiStatusNM::getVisibleAPs() {
    const ScopedLock lock(wifiLock);
    NMDeviceWifi* wifiDevice = NM_DEVICE_WIFI(nmDevice);
    const GPtrArray* apList = nm_device_wifi_get_access_points(wifiDevice);
    Array<WifiAccessPoint> accessPoints;

    if (apList != nullptr)
    {
        std::map<String, WifiAccessPoint> uniqueAPs;
        for (int i = 0; i < apList->len; i++)
        {
            NMAccessPoint *ap = (NMAccessPoint *) g_ptr_array_index(apList, i);
            WifiAccessPoint createdAP = createNMWifiAccessPoint(ap);

            /*FIXME: dropping hidden (no ssid) networks until gui supports it*/
            if (createdAP.getSSID().length() == 0)
            {
                continue;
            }

            if (uniqueAPs.count(createdAP.getHash()) == 0)
            {
                uniqueAPs[createdAP.getHash()] = createdAP;
            }
            else
            {
                if (createdAP.getSignalStrength() <
                    uniqueAPs[createdAP.getHash()].getSignalStrength())
                {
                    uniqueAPs[createdAP.getHash()] = createdAP;
                }
            }
        }
        for (const auto ap : uniqueAPs)
        {
            accessPoints.add(ap.second);
        }
    }
    DBG("WifiStatusNM::" << __func__ << ": found " << accessPoints.size()
            << " AccessPoints");
    return accessPoints;
}

/**
 */
void WifiStatusNM::connectToAccessPoint(WifiAccessPoint toConnect,
        String psk = String()) { }

/**
 */
void WifiStatusNM::disconnect() { }

/**
 * Turns on the wifi radio.
 */
void WifiStatusNM::enableWifi()
{
    const ScopedLock lock(wifiLock);
    if (!isWifiDeviceEnabled())
    {
        DBG("WifiStatusNM::" << __func__ << ": enabling wifi connections");
        nm_client_wireless_set_enabled(nmClient, true);
        if (!isThreadRunning())
        {
            startThread();
        }
    }
    else
    {
        DBG("WifiStatusNM::" << __func__ << ": wifi is already enabled!");
    }
}

/**
 * Turns off the wifi radio.
 */
void WifiStatusNM::disableWifi()
{
    const ScopedLock lock(wifiLock);
    if (isWifiDeviceEnabled())
    {
        DBG("WifiStatusNM::" << __func__ << ": disabling wifi connections");
        nm_client_wireless_set_enabled(nmClient, false);
    }
    else
    {
        DBG("WifiStatusNM::" << __func__ << ": wifi is already disabled!");
    }
}

WifiStatusNM::~WifiStatusNM() { }

bool WifiStatusNM::isEnabled() const
{
    const ScopedLock lock(wifiLock);
    return enabled;
}
/**
 * Attempts to connect to a wifi access point.
 */
void WifiStatusNM::setConnectedAccessPoint
(const WifiAccessPoint& ap, String psk)
{
    const ScopedLock lock(wifiLock);
    DBG("WifiStatusNM::" << __func__ << ": trying to connect to "
            << ap.getSSID());
    notifyListeners(wifiBusy);
    const char* apPath = nullptr;
    const GPtrArray* apList = nm_device_wifi_get_access_points
            (NM_DEVICE_WIFI(nmDevice));
    if (apList == nullptr)
    {
        DBG("WifiStatusNM::" << __func__
                << ": can't connect, no access points found.");
        return;
    }
    NMAccessPoint* candidateAP = nullptr;
    for (int i = 0; i < apList->len; i++)
    {
        candidateAP = (NMAccessPoint*) g_ptr_array_index(apList, i);
        const char* candidateHash = hashAP(candidateAP);
        if (ap.getHash() == candidateHash)
        {
            apPath = nm_object_get_path(NM_OBJECT(candidateAP));
            break;
        }
    }
    if (!apPath)
    {
        DBG("WifiStatusNM::" << __func__
                << ": no matching access point in list of length "
                << String(apList->len));
        return;
    }
    connecting = true;
    DBG("WifiStatusNM::" << __func__ << ": matching NMAccessPoint found");
    NMConnection* connection = nm_connection_new();
    NMSettingWireless* settingWifi = (NMSettingWireless*)
            nm_setting_wireless_new();
    nm_connection_add_setting(connection, NM_SETTING(settingWifi));
    g_object_set(settingWifi,
            NM_SETTING_WIRELESS_SSID,
            nm_access_point_get_ssid(candidateAP),
            NM_SETTING_WIRELESS_HIDDEN,
            false,
            nullptr);

    if (!psk.isEmpty())
    {
        NMSettingWirelessSecurity* settingWifiSecurity =
                (NMSettingWirelessSecurity*)
                nm_setting_wireless_security_new();
        nm_connection_add_setting(connection, NM_SETTING(settingWifiSecurity));

        if (nm_access_point_get_wpa_flags(candidateAP)
            == NM_802_11_AP_SEC_NONE
            && nm_access_point_get_rsn_flags(candidateAP)
            == NM_802_11_AP_SEC_NONE)
        {
            DBG("WifiStatusNM::" << __func__
                    << ": access point has WEP security");
            /* WEP */
            nm_setting_wireless_security_set_wep_key(settingWifiSecurity,
                    0, psk.toRawUTF8());
            if (isValidWEPKeyFormat(psk))
            {
                g_object_set(G_OBJECT(settingWifiSecurity),
                        NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE,
                        NM_WEP_KEY_TYPE_KEY, nullptr);
            }
            else if (isValidWEPPassphraseFormat(psk))
            {
                g_object_set(G_OBJECT(settingWifiSecurity),
                        NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE,
                        NM_WEP_KEY_TYPE_PASSPHRASE, nullptr);
            }
            else
            {
                DBG("WifiStatusNM::" << __func__
                        << ": User input invalid WEP Key type, psk.length() = "
                        << psk.length() << ", not in [5,10,13,26]");
            }
        }
        else
        {
            DBG("WifiStatusNM::" << __func__
                    << ": access point has WPA security");
            g_object_set(settingWifiSecurity, NM_SETTING_WIRELESS_SECURITY_PSK,
                    psk.toRawUTF8(), nullptr);
        }
    }
    nm_client_add_and_activate_connection(nmClient,
            connection,
            nmDevice,
            apPath,
            handle_add_and_activate_finish,
            this);
}

/**
 * If connected to a wifi access point, this will close that connection.
 */
void WifiStatusNM::disconnect()
{
    const ScopedLock lock(wifiLock);
    if (connected)
    {
        DBG("WifiStatusNM::" << __func__ << " disconnecting from "
                << connectedAP.getSSID());
        nm_device_disconnect(nmDevice, nullptr, nullptr);
        notifyListeners(wifiBusy);
        connected = false;
    }
    else
    {
        DBG("WifiStatusNM::" << __func__
                << " trying to disconnect, but there is no connection.");
    }
}

/**
 * Inform all listeners when wifi is enabled or disabled
 */
void WifiStatusNM::handleWirelessEnabled()
{
    enabled = nm_client_wireless_get_enabled(nmClient);
    DBG("WifiStatusNM::" << __func__ << ":  changed to "
            << (enabled ? "enabled" : "disabled"));
    //FIXME: Force and wait for a scan after enable
    if (enabled)
    {
        notifyListeners(wifiEnabled);
    }
    else
    {
        notifyListeners(wifiDisabled);
        signalThreadShouldExit();
        notify();
    }
}

/**
 * Inform all listeners when a wifi connection is created or fails
 * to be created.
 */
void WifiStatusNM::handleWirelessConnected()
{
    NMDeviceState state = nm_device_get_state(nmDevice);
    DBG("WifiStatusNM::" << __func__ << ":  changed to " << String(state)
            << " while connecting = " << (connecting ? "true" : "false"));
    switch (state)
    {
        case NM_DEVICE_STATE_ACTIVATED:
            if (connected)
            {
                break;
            }
            handle_active_access_point(this);
            connected = true;
            connecting = false;
            DBG("WifiStatusNM::" << __func__ << ": connected");
            notifyListeners(wifiConnected);
            break;
        case NM_DEVICE_STATE_PREPARE:
        case NM_DEVICE_STATE_CONFIG:
        case NM_DEVICE_STATE_IP_CONFIG:
        case NM_DEVICE_STATE_IP_CHECK:
        case NM_DEVICE_STATE_SECONDARIES:
            /* No state change for now, wait for connection to complete/fail */
            break;
        case NM_DEVICE_STATE_NEED_AUTH:
            if (connecting)
            {
                DBG("WifiStatusNM::" << __func__
                        << " missing auth, cancelling connection.");
                NMActiveConnection *conn =
                        nm_client_get_activating_connection(nmClient);
                removeNMConnection(conn);
            }
            /* FIXME: let this drop into the general failed case for now
             *        eventually this should prompt the user
             */
        case NM_DEVICE_STATE_DISCONNECTED:
        case NM_DEVICE_STATE_DEACTIVATING:
        case NM_DEVICE_STATE_FAILED:
            if (connecting)
            {
                connected = false;
                connecting = false;
                DBG("WifiStatusNM::" << __func__ << ": failed");
                notifyListeners(wifiConnectionFailed);
                break;
            }
            if (!connected)
            {
                break;
            }
            handle_active_access_point(this);
            connected = false;
            notifyListeners(wifiDisconnected);
            break;
        case NM_DEVICE_STATE_UNKNOWN:
        case NM_DEVICE_STATE_UNMANAGED:
        case NM_DEVICE_STATE_UNAVAILABLE:
        default:
            if (connecting || connected)
            {
                DBG("WifiStatusNM::" << __func__
                        << ": wlan0 device entered unmanaged state: " << state);
                handle_active_access_point(this);
                connected = false;
                connecting = false;
                notifyListeners(wifiDisconnected);
            }
    }
}

/**
 * Save access point data after connecting to an access point.
 */
void WifiStatusNM::handleConnectedAccessPoint()
{
    DBG("WifiStatusNM::" << __func__ << ":  changed active AP");
    connectedAP = getNMConnectedAP(NM_DEVICE_WIFI(nmDevice));
    if (connectedAP.isNull())
    {
        DBG("WifiStatusNM::" << __func__ << ": no connected AP");
        connected = false;
        notifyListeners(wifiDisconnected);
    }
    else
    {
        DBG("WifiStatusNM::" << __func__ << ":  ssid = "
                << connectedAP.getSSID());
    }
}

/**
 * Attempt to open a connection to the network manager.
 * @return true iff the connection was made and nmClient was initialized.
 */
bool WifiStatusNM::connectToNetworkManager()
{
    if (!nmClient || !NM_IS_CLIENT(nmClient))
    {
        nmClient = nm_client_new();
    }
    if (!nmClient || !NM_IS_CLIENT(nmClient))
    {
        DBG("WifiStatusNM::" << __func__
                << ": failed to connect to nmclient over dbus");
    }
    if (!nmDevice || !NM_IS_DEVICE(nmDevice))
    {
        nmDevice = nm_client_get_device_by_iface(nmClient, "wlan0");
    }
    if (!nmDevice || !NM_IS_DEVICE(nmDevice))
    {

        DBG("WifiStatusNM::" << __func__ <<
                ":  failed to connect to nmdevice wlan0 over dbus");
    }
    return nmClient != nullptr;
}

/**
 * While the wifi status thread is running, regularly check the network
 * manager for wifi network changes.
 */
void WifiStatusNM::run()
{
    if (nmClient == nullptr)
    {
        DBG("WifiStatusNM::" << __func__
                << ": NetworkManager client is null, exiting wifi thread");
        enabled = false;
        notifyListeners(wifiDisabled);
        return;
    }
    NMDevice* dev = nm_client_get_device_by_iface(nmClient, "wlan0");
    if (dev == nullptr)
    {
        DBG("WifiStatusNM::" << __func__
                << ": device wlan0 is null, exiting wifi thread");
        enabled = false;
        notifyListeners(wifiDisabled);
        return;
    }
    while (!threadShouldExit())
    {
        {
            const MessageManagerLock mmLock;
            bool dispatched = g_main_context_iteration(context, false);
        }
        wait(LIBNM_ITERATION_PERIOD);
    }
}

/**
 * Generate a unique hash string identifying a wifi access point.
 * Adapted from network-manager-applet src/utils/utils.c 
 */
char* WifiStatusNM::hashAP(NMAccessPoint* ap)
{
    const GByteArray* ssid = nm_access_point_get_ssid(ap);
    NM80211Mode mode = nm_access_point_get_mode(ap);
    guint32 flags = nm_access_point_get_flags(ap);
    guint32 wpa_flags = nm_access_point_get_wpa_flags(ap);
    guint32 rsn_flags = nm_access_point_get_rsn_flags(ap);
    unsigned char input[66];

    memset(&input[0], 0, sizeof (input));

    if (ssid)
    {
        memcpy(input, ssid->data, ssid->len);
    }

    if (mode == NM_802_11_MODE_INFRA)
    {
        input[32] |= (1 << 0);
    }
    else if (mode == NM_802_11_MODE_ADHOC)
    {
        input[32] |= (1 << 1);
    }
    else
    {
        input[32] |= (1 << 2);
    }

    /* Separate out no encryption, WEP-only, and WPA-capable */
    if (!(flags & NM_802_11_AP_FLAGS_PRIVACY)
        && (wpa_flags == NM_802_11_AP_SEC_NONE)
        && (rsn_flags == NM_802_11_AP_SEC_NONE))
    {
        input[32] |= (1 << 3);
    }
    else if ((flags & NM_802_11_AP_FLAGS_PRIVACY)
             && (wpa_flags == NM_802_11_AP_SEC_NONE)
             && (rsn_flags == NM_802_11_AP_SEC_NONE))
    {
        input[32] |= (1 << 4);
    }
    else if (!(flags & NM_802_11_AP_FLAGS_PRIVACY)
             && (wpa_flags != NM_802_11_AP_SEC_NONE)
             && (rsn_flags != NM_802_11_AP_SEC_NONE))
    {
        input[32] |= (1 << 5);
    }
    else
    {
        input[32] |= (1 << 6);
    }

    /* duplicate it */
    memcpy(&input[33], &input[0], 32);

    return g_compute_checksum_for_data(G_CHECKSUM_MD5, input, sizeof (input));
}

/**
 * @return true iff ap is a secure access point.
 */
bool WifiStatusNM::resolveAPSecurity(NMAccessPoint *ap)
{
    //FIXME: Assumes all security types equal

    return (
            nm_access_point_get_flags(ap) == NM_802_11_AP_FLAGS_PRIVACY ||
            nm_access_point_get_wpa_flags(ap) != NM_802_11_AP_SEC_NONE ||
            nm_access_point_get_rsn_flags(ap) != NM_802_11_AP_SEC_NONE
            );
}

/**
 * @return WifiAccessPoint data read from a NMAccessPoint
 */
WifiAccessPoint WifiStatusNM::createNMWifiAccessPoint(NMAccessPoint *ap)
{
    const GByteArray *ssid = nm_access_point_get_ssid(ap);
    //GBytes *ssid = nm_access_point_get_ssid(ap);
    char *ssid_str = nullptr, *ssid_hex_str = nullptr;
    bool security = resolveAPSecurity(ap);

    /* Convert to strings */
    if (ssid)
    {
        const guint8 *ssid_data = ssid->data;
        gsize ssid_len = ssid->len;

        //ssid_data = (const guint8 *) g_bytes_get_data(ssid, &ssid_len);
        ssid_str = nm_utils_ssid_to_utf8(ssid);
    }

    if (!ssid_str || !ssid)
    {
        DBG("WifiStatusNM::" << __func__
                << ": libnm conversion of ssid to utf8 failed, using empty string");
    }

    return WifiAccessPoint(!ssid_str ? "" : ssid_str,
            nm_access_point_get_strength(ap),
            security,
            hashAP(ap));
}

/**
 * @return WifiAccessPoint data of the access point connected to wdev, or
 * WifiAccessPoint() if no connection is found.
 */
WifiAccessPoint WifiStatusNM::getNMConnectedAP(NMDeviceWifi *wdev)
{
    NMAccessPoint *ap = nm_device_wifi_get_active_access_point(wdev);

    if (!wdev || !ap)
    {
        DBG("WifiStatusNM::" << __func__ << ": no NMAccessPoint found!");

        return WifiAccessPoint();
    }

    return createNMWifiAccessPoint(ap);
}

/**
 * Close all connections on wifi device wlan0.
 */
void WifiStatusNM::removeNMConnection(NMActiveConnection *conn)
{
    if (nmDevice == nullptr)
    {
        DBG("WifiStatusNM::" << __func__ << ": wlan0 is missing!");
        return;
    }
    if (conn == nullptr)
    {
        DBG("WifiStatusNM::" << __func__ << ": connection is null!");
        return;
    }
    const char *ac_uuid = nm_active_connection_get_uuid(conn);
    const GPtrArray *avail_cons = nm_device_get_available_connections
            (nmDevice);
    for (int i = 0; avail_cons && (i < avail_cons->len); i++)
    {
        NMRemoteConnection* candidate = (NMRemoteConnection*)
                g_ptr_array_index(avail_cons, i);
        const char* test_uuid = nm_connection_get_uuid
                (NM_CONNECTION(candidate));

        if (g_strcmp0(ac_uuid, test_uuid) == 0)
        {
            GError *err = nullptr;
            nm_remote_connection_delete(candidate, nullptr, &err);
            if (err)
            {
                DBG("WifiStatusNM::" << __func__
                        << ": failed to remove active connection!");
                DBG("WifiStatusNM::" << __func__ << ": "
                        << err->message);
                g_error_free(err);
            }
            break;
        }
    }
}

/**
 * @param key
 * @return true iff key has the correct format for a WEP key.
 */
bool WifiStatusNM::isValidWEPKeyFormat(String key)
{

    return (key.length() == 10) || (key.length() == 26);
}

/**
 * @param phrase
 * @return true iff phrase has the correct format for a WEP passphrase.
 */
bool WifiStatusNM::isValidWEPPassphraseFormat(String phrase)
{

    return (phrase.length() == 5) || (phrase.length() == 13);
}


//##############  NetworkManager callback functions ###########################
// These are called by the NetworkManager after wifi state changes.

void WifiStatusNM::handle_add_and_activate_finish(NMClient *client,
        NMActiveConnection *active,
        const char* path,
        GError *err,
        gpointer user_data)
{
    WifiStatusNM *wifiStatus = (WifiStatusNM *) user_data;
    const ScopedLock lock(wifiStatus->wifiLock);
    if (err)
    {
        DBG("WifiStatusNM::" << __func__
                << ": failed to add/activate connection!");
        DBG("WifiStatusNM::" << __func__ << ": " << err->message);
        wifiStatus->handleWirelessConnected();
    }
}

void WifiStatusNM::handle_wireless_enabled(WifiStatusNM *wifiStatus)
{
    const ScopedLock lock(wifiStatus->wifiLock);
    DBG("WifiStatusNM::" << __func__ << ": SIGNAL: "
            << NM_CLIENT_WIRELESS_ENABLED << ": changed! ");
    if (wifiStatus->isEnabled())
    {
        wifiStatus->handleWirelessEnabled();
    }
}

void WifiStatusNM::handle_wireless_connected(WifiStatusNM *wifiStatus)
{
    const ScopedLock lock(wifiStatus->wifiLock);
    DBG("WifiStatusNM::" << __func__ << ": SIGNAL: "
            << NM_DEVICE_STATE << ": changed! ");
    wifiStatus->handleWirelessConnected();
}

void WifiStatusNM::handle_active_access_point(WifiStatusNM *wifiStatus)
{
    const ScopedLock lock(wifiStatus->wifiLock);
    DBG("WifiStatusNM::" << __func__ << ": SIGNAL: "
            << NM_DEVICE_WIFI_ACTIVE_ACCESS_POINT << ": changed! ");
    wifiStatus->handleConnectedAccessPoint();
}

void WifiStatusNM::handle_changed_access_points(WifiStatusNM *wifiStatus)
{
    const ScopedLock lock(wifiStatus->wifiLock);
    DBG("WifiStatusNM::" << __func__
            << ": SIGNAL: access-point-added | access-point-removed: changed! ");
    if (!wifiStatus->isEnabled())
    {
        wifiStatus->handleWirelessEnabled();
    }
}

#endif // LINUX
