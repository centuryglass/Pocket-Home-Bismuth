#include "LibNMHandler.h"
#include <nm-remote-connection.h>
#include "WifiAccessPoint.h"

/**
 * Loads client and device objects, and starts the signal thread.
 */
LibNMHandler::LibNMHandler() : 
enabledChangeCallback([](bool enabled){}),
apUpdateCallback([](Array<WifiAccessPoint> aps){}),
stateUpdateCallback([](NMDeviceState s){}),
connectionUpdateCallback([](WifiAccessPoint w){})        
{
    GLibSignalHandler signalHandler;
    signalHandler.gLibCall([this]()
    {
        nmClient = nm_client_new();
        if (nmClient == nullptr || !NM_IS_CLIENT(nmClient))
        {
            DBG("WifiEventHandler::" << __func__
                    << ": failed to connect to nmclient over dbus");
                    nmClient = nullptr;
            return;
        }

        //find wifi device
        const GPtrArray* allDevices = nm_client_get_devices(nmClient);
        for (int i = 0; i < allDevices->len; i++)
        {
            if (NM_IS_DEVICE_WIFI(allDevices->pdata[i]))
            {
                nmDevice = (NMDevice*) allDevices->pdata[i];
                        nmWifiDevice = NM_DEVICE_WIFI(nmDevice);
                break;
            }
        }
        nmDevice = nm_client_get_device_by_iface(nmClient, "wlan0");
        if (nmDevice == nullptr || !NM_IS_DEVICE_WIFI(nmDevice))
        {
            DBG("WifiEventHandler::" << __func__ <<
                    ":  failed to find a wifi device!");
                    nmClient = nullptr;
                    nmDevice = nullptr;
                    nmWifiDevice = nullptr;
        }
    });
}

/**
 * Shuts down the signal thread, removing all signal handlers.
 */
LibNMHandler::~LibNMHandler()
{
    disconnectSignalHandlers();
}

/**
 * Checks if the network manager and the wifi device are available
 */
bool LibNMHandler::isWifiAvailable()
{
    return nmClient != nullptr
            && nmDevice != nullptr
            && nmWifiDevice != nullptr;
}

    
/**
 * @return true iff the wifi device is enabled. 
 */
bool LibNMHandler::checkWifiEnabled()
{
    bool enabled = false;
    if(isWifiAvailable())
    {
        GLibSignalHandler signalHandler;
        signalHandler.gLibCall([this, &enabled]()
        {
            enabled = nm_client_wireless_get_enabled(nmClient);
        });
    }
    return enabled;
}


/**
 * @return true iff the wifi device is connecting to an access point. 
 */
bool LibNMHandler::checkWifiConnecting()
{
    bool connecting = false;
    if(isWifiAvailable())
    {
        GLibSignalHandler signalHandler;
        signalHandler.gLibCall([this, &connecting]()
        {  
            connecting = isWifiConnection
                (nm_client_get_activating_connection(nmClient));
        });
    }
    return connecting;
}

/**
 * @return true iff the wifi device is connected to an access point. 
 */
bool LibNMHandler::checkWifiConnected()
{
    bool connected = false;
    if(isWifiAvailable())
    {
        GLibSignalHandler signalHandler;
        signalHandler.gLibCall([this, &connected]()
        {  
            connected = isWifiConnection
                (nm_client_get_primary_connection(nmClient));
        });
    }
    return connected;
}

/**
 * Gets the current connected access point from the network manager wifi
 * device.
 * 
 * @return the current connected access point, or the void access point if
 *         none is found.
 */
WifiAccessPoint LibNMHandler::findConnectedAP()
{
    WifiAccessPoint ap;
    if(isWifiAvailable())
    {
        GLibSignalHandler signalHandler;
        signalHandler.gLibCall([this, &ap]()
        {
            NMAccessPoint* nmAP = nm_device_wifi_get_active_access_point
                    (nmWifiDevice);
            if(nmAP != nullptr)
            {
                ap = WifiAccessPoint(nmAP);
            }
        });
    }
    return ap;
}


/**
 * Gets the current connecting access point from the network manager wifi
 * device.
 * 
 * @return the current connecting access point, or the void access point if
 *         none is found.
 */
WifiAccessPoint LibNMHandler::findConnectingAP()
{
    WifiAccessPoint ap;
    if(isWifiAvailable())
    {
        GLibSignalHandler signalHandler;
        signalHandler.gLibCall([this, &ap]()
        {
            NMActiveConnection* conn = nm_client_get_activating_connection
                    (nmClient);
            if (conn != nullptr)
            {
                NMAccessPoint* connectingAP =
                        nm_device_wifi_get_access_point_by_path(
                        nmWifiDevice,
                        nm_active_connection_get_specific_object(conn));
                if(connectingAP != nullptr)
                {
                    ap = WifiAccessPoint(connectingAP);
                }
            }
        });
    }
    return ap;
}

/**
 * Updates and returns the list of visible wifi access points.
 */
Array<WifiAccessPoint> LibNMHandler::updatedVisibleAPs()
{
    Array<WifiAccessPoint> visibleAPs;
    if(isWifiAvailable())
    {
        buildAPMap();
        GLibSignalHandler signalHandler;
        signalHandler.gLibCall([this, &visibleAPs]()
        {
            for(auto it = accessPointMap.begin();
                it != accessPointMap.end(); it++)
            {
                WifiAccessPoint wifiAP = it->first;
                Array<NMAccessPoint*> nmAPs = it->second;
                int bestStrength = -1;
                for(NMAccessPoint* nmAP : nmAPs)
                {
                    int strength = nm_access_point_get_strength(nmAP);
                    if(strength > bestStrength)
                    {
                        bestStrength = strength;
                    }
                }
                if(bestStrength >= 0)
                {
                    wifiAP.setSignalStrength(bestStrength);
                    visibleAPs.add(wifiAP);
                }
            }
        });
    }
    return visibleAPs;
}

/**
 * Turns the wifi device on or off.
 * 
 * @param wifiEnabled  If true, wifi will be enabled.  If false, wifi will
 *                     be disabled.
 */
void LibNMHandler::setWifiEnabled(bool wifiEnabled)
{
    if(isWifiAvailable())
    {
        GLibSignalHandler signalHandler;
        signalHandler.gLibCall([this, wifiEnabled]()
        {
            nm_client_wireless_set_enabled(nmClient, wifiEnabled);
        });
    }
}

/**
 * Send a request to the wifi device to scan for new access points.
 */
void LibNMHandler::requestScan()
{
    if(isWifiAvailable())
    {
        GLibSignalHandler signalHandler;
        signalHandler.gLibCallAsync([this]()
        {
            GError* error = nullptr;
            nm_device_wifi_request_scan_simple(nmWifiDevice,nullptr,error);
            if(error != nullptr);
            {
                DBG("LibNMHandler::requestScan: error requesting scan: "
                        << String(error->message));
                g_error_free(error);
            }
        });
    }
}

    
/**
 * @return  the current state of the wifi device. 
 */
NMDeviceState LibNMHandler::findWifiState()
{
    NMDeviceState state = NM_DEVICE_STATE_UNAVAILABLE;
    if(isWifiAvailable())
    {
        GLibSignalHandler signalHandler;
        signalHandler.gLibCall([this, &state]()
        {
            state = nm_device_get_state(nmDevice);
        });
    }
    return state;
}

//##### Static data for LibNMHandler::initConnection: ########
struct ConnectingCallbackData
{
    std::function<void(WifiAccessPoint) > connectingCallback;
    std::function<void() > failureCallback;
    NMDeviceWifi* wifiDevice;
};

//libNM callback to run after attempting to open a new connection

static void handleConnectionAttempt(
        NMClient *client,
        NMActiveConnection *active,
        const char* path,
        GError *err,
        ConnectingCallbackData* callbackData)
{
    g_assert(g_main_context_is_owner(g_main_context_default()));
    NMAccessPoint* ap =
            nm_device_wifi_get_access_point_by_path(
            callbackData->wifiDevice,
            nm_active_connection_get_specific_object(active));
    if (err != nullptr || ap == nullptr)
    {
        callbackData->failureCallback();
        g_error_free(err);
    }
    else
    {
        callbackData->connectingCallback(WifiAccessPoint(ap));
    }
}
//libNM callback run after attempting to re-open a known connection

static void handleKnownConnectionAttempt(
        NMClient *client,
        NMActiveConnection *active,
        GError *err,
        ConnectingCallbackData* callbackData)
{
    handleConnectionAttempt(client, active, nullptr, err, callbackData);
}

/**
 * Attempts to open a connection to an access point.
 */
void LibNMHandler::initConnection(
        const WifiAccessPoint& toConnect,
        String psk,
        std::function<void(WifiAccessPoint) > connectingCallback,
        std::function<void() > failureCallback)
{
    //Set callback functions to run asynchronously on the message thread and 
    //then delete callback data.
    ConnectingCallbackData* callbackData = new ConnectingCallbackData;
    callbackData->connectingCallback = [connectingCallback, callbackData]
            (WifiAccessPoint ap)
    {
        MessageManager::callAsync([callbackData, &ap]
        {
            callbackData->connectingCallback(ap);
            delete callbackData;
        });
    };
    callbackData->failureCallback = [failureCallback, callbackData]()
    {
        MessageManager::callAsync([callbackData]
        {
            callbackData->failureCallback();
            delete callbackData;
        });
    };
    callbackData->wifiDevice = nmWifiDevice;

    if (!isWifiAvailable())
    {
        callbackData->failureCallback();
        return;
    }
    GLibSignalHandler signalHandler;
    signalHandler.gLibCall([this, &toConnect, &psk, &callbackData]()
    {
        g_assert(g_main_context_is_owner(g_main_context_default()));
        NMConnection * connection = nullptr;

        //find the best matching access point for the connection.
        NMAccessPoint* matchingAP = nullptr;
        int bestSignalStrength = -1;
        Array<NMAccessPoint*>& matches = accessPointMap[toConnect];
        for (NMAccessPoint* candidate : matches)
        {
            int signalStrength = nm_access_point_get_strength(candidate);
            if (signalStrength > bestSignalStrength)
            {
                bestSignalStrength = signalStrength;
                        matchingAP = candidate;
            }
        }
        if (matchingAP == nullptr)
        {
            callbackData->failureCallback();
            return;
        }

        //Check for existing connections:
        const GPtrArray* wifiConnections = nm_device_get_available_connections
                (nmDevice);
        GSList* wifiConnList = nullptr;
        for(int i = 0; i < wifiConnections->len; i++)
        {
            wifiConnList = g_slist_prepend(wifiConnList,
                    wifiConnections->pdata[i]);
        }
        GSList* matchingConnections = nm_access_point_filter_connections
                (matchingAP, wifiConnList);
        if (matchingConnections != nullptr)
        {
            connection = (NMConnection*) matchingConnections->data;
        }
        g_slist_free(wifiConnList);
        g_slist_free(matchingConnections);
        matchingConnections = nullptr;
        
        //Create a new connection if no existing one was found:
        if (connection == nullptr)
        {
            connection = nm_connection_new();
            NMSettingWireless* wifiSettings
                    = (NMSettingWireless*) nm_setting_wireless_new();
            nm_connection_add_setting(connection, NM_SETTING(wifiSettings));
            g_object_set(wifiSettings,
                    NM_SETTING_WIRELESS_SSID,
                    nm_access_point_get_ssid(matchingAP),
                    NM_SETTING_WIRELESS_HIDDEN,
                    false,
                    nullptr);
        }

        //If a password is provided, save it to the connection.
        if (!psk.isEmpty() && (!toConnect.isSavedConnection() 
                || !toConnect.hasSavedPsk()))
        {
            NMSettingWirelessSecurity* settingWifiSecurity
                    = (NMSettingWirelessSecurity*)
                    nm_setting_wireless_security_new();
                    nm_connection_add_setting(connection,
                    NM_SETTING(settingWifiSecurity));

            if (nm_access_point_get_wpa_flags(matchingAP)
                == NM_802_11_AP_SEC_NONE
                && nm_access_point_get_rsn_flags(matchingAP)
                == NM_802_11_AP_SEC_NONE)
            {
                DBG("LibNMHandler::" << __func__
                        << ": access point has WEP security");
                        /* WEP */
                        nm_setting_wireless_security_set_wep_key
                                (settingWifiSecurity,0, psk.toRawUTF8());
                        //valid key format: length 10 or length 26
                if (psk.length() == 10 || psk.length() == 26)
                {
                    g_object_set(G_OBJECT(settingWifiSecurity),
                            NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE,
                            NM_WEP_KEY_TYPE_KEY, nullptr);
                }
                    //valid passphrase format: length 5 or length 14
                else if (psk.length() == 5 || psk.length() == 13)
                {
                    g_object_set(G_OBJECT(settingWifiSecurity),
                            NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE,
                            NM_WEP_KEY_TYPE_PASSPHRASE, nullptr);
                }
                else
                {
                    DBG("LibNMHandler::" << __func__
                            << ": User input invalid WEP Key type, "
                            << "psk.length() = " << psk.length()
                            << ", not in [5,10,13,26]");
                }
            }
            else
            {
                DBG("LibNMHandler::" << __func__
                        << ": access point has WPA security");
                        g_object_set(settingWifiSecurity,
                        NM_SETTING_WIRELESS_SECURITY_PSK,
                        psk.toRawUTF8(), nullptr);
            }
        }
        if(toConnect.isSavedConnection())
        {   
            nm_client_activate_connection(nmClient,
                    connection,
                    nmDevice,
                    nm_object_get_path(NM_OBJECT(matchingAP)),
                    (NMClientActivateFn) handleKnownConnectionAttempt,
                    callbackData);
        }
        else{
            nm_client_add_and_activate_connection(nmClient,
                    connection,
                    nmDevice,
                    nm_object_get_path(NM_OBJECT(matchingAP)),
                    (NMClientAddActivateFn) handleConnectionAttempt,
                    callbackData);
        }
    });
}

/**
 * Shuts down the active wifi connection.
 */
void LibNMHandler::closeActiveConnection()
{  
    if(isWifiAvailable())
    {
        GLibSignalHandler signalHandler;
        signalHandler.gLibCall([this]()
        {
                nm_device_disconnect(nmDevice, nullptr, nullptr);
        });
    }
}


    
/**
 * Shuts down any wifi connection currently being activated.
 */
void LibNMHandler::closeActivatingConnection()
{  
    if(isWifiAvailable())
    {
        GLibSignalHandler signalHandler;
        signalHandler.gLibCall([this]()
        {
            NMActiveConnection *conn = nm_client_get_activating_connection
                    (nmClient);
            if(conn != nullptr)
            {
                const char *ac_uuid = nm_active_connection_get_uuid(conn);
                const GPtrArray* avail_cons = 
                        nm_device_get_available_connections(nmDevice);
                for (int i = 0; avail_cons && (i < avail_cons->len); i++)
                {
                    NMRemoteConnection* candidate = (NMRemoteConnection*)
                            g_ptr_array_index(avail_cons, i);
                    const char* test_uuid = nm_connection_get_uuid
                            (NM_CONNECTION(candidate));
                    if(g_strcmp0(ac_uuid, test_uuid) == 0)
                    {
                        GError *err = nullptr;
                        nm_remote_connection_delete(candidate, nullptr, &err);
                        if (err)
                        {
                            DBG("LibNMInterface::" << __func__
                                    << ": failed to remove active connection!");
                            DBG("LibNMInterface::" << __func__ << ": "
                                    << err->message);
                            g_error_free(err);
                        }
                        break;
                    }
                }        
            }
        });
    }
}

    
/**
 * Attach all signal handlers to the wifi thread, so that they are run
 * whenever the appropriate signals occur.
 */
void LibNMHandler::connectSignalHandlers()
{
    //Signal: notifies that wifi has turned on or off
    nmClientSignalConnect("notify::" NM_CLIENT_WIRELESS_ENABLED,
            G_CALLBACK(handleWifiEnabledChange), this);

    //Signal: notifies that wifi state has changed
    nmDeviceSignalConnect("notify::" NM_DEVICE_STATE,
            G_CALLBACK(handleStateChange), this);

    //Signal: notifies that the active access point has changed
    nmWifiDeviceSignalConnect("notify::" NM_DEVICE_WIFI_ACTIVE_ACCESS_POINT,
            G_CALLBACK(handleConnectionChange), this);

    //Signal: notifies that a new wifi access point is visible.
    nmWifiDeviceSignalConnect("access-point-added",
            G_CALLBACK(handleApAdded), this);

    //Signal: notifies that a wifi access point is no longer visible.
    nmWifiDeviceSignalConnect("access-point-removed",
            G_CALLBACK(handleApRemoved), this);
}

/**
 * Remove all signal handlers from the wifi thread, so that they don't 
 * register wifi updates.
 */
void LibNMHandler::disconnectSignalHandlers()
{
    if (nmWifiDevice != nullptr && NM_IS_DEVICE_WIFI(nmWifiDevice))
    {
        for (gulong& signalHandlerId : wifiDeviceSignalHandlers)
        {
            g_signal_handler_disconnect(nmWifiDevice, signalHandlerId);
        }
    }
    wifiDeviceSignalHandlers.clear();

    if (nmDevice != nullptr && NM_IS_DEVICE(nmDevice))
    {
        for (gulong& signalHandlerId : deviceSignalHandlers)
        {
            g_signal_handler_disconnect(nmDevice, signalHandlerId);
        }
    }
    deviceSignalHandlers.clear();

    if (nmClient != nullptr && NM_IS_CLIENT(nmClient))
    {
        for (gulong& signalHandlerId : clientSignalHandlers)
        {
            g_signal_handler_disconnect(nmClient, signalHandlerId);
        }
        clientSignalHandlers.clear();
    }
    clientSignalHandlers.clear();
}


//Internal signal handlers:
void LibNMHandler::handleWifiEnabledChange
(NMClient* client, LibNMHandler* nmHandler)
{
    g_assert(g_main_context_is_owner(g_main_context_default()));
    MessageManager::callAsync([nmHandler]()
    {
        nmHandler->enabledChangeCallback(nmHandler->checkWifiEnabled());
    }); 
    
}

void LibNMHandler::handleStateChange
(NMDevice* device, LibNMHandler* nmHandler)
{   
    g_assert(g_main_context_is_owner(g_main_context_default()));
    NMDeviceState state = nm_device_get_state(device);
    MessageManager::callAsync([nmHandler,state]()
    {
        nmHandler->stateUpdateCallback(state);
    });  
}

void LibNMHandler::handleApAdded
(NMDeviceWifi* wifiDevice, LibNMHandler* nmHandler)
{   
    g_assert(g_main_context_is_owner(g_main_context_default()));
    nmHandler->buildAPMap(); 
    MessageManager::callAsync([nmHandler]()
    {
        nmHandler->apUpdateCallback(nmHandler->updatedVisibleAPs());
    }); 
    
}

void LibNMHandler::handleApRemoved
(NMDeviceWifi* wifiDevice, LibNMHandler* nmHandler)
{    
    g_assert(g_main_context_is_owner(g_main_context_default()));
    nmHandler->accessPointMap.clear();
    nmHandler->handleApAdded(wifiDevice,nmHandler);
    
}

void LibNMHandler::handleConnectionChange
(NMDeviceWifi* wifiDevice, LibNMHandler* nmHandler)
{ 
    g_assert(g_main_context_is_owner(g_main_context_default()));
    WifiAccessPoint connected = nmHandler->findConnectedAP();
    MessageManager::callAsync([nmHandler, connected]()
    {
        nmHandler->connectionUpdateCallback(connected);
    }); 
}

/**
 * Checks if a connection belongs to the wifi device.  This function should
 * be called from the GLib event thread.
 */
bool LibNMHandler::isWifiConnection(NMActiveConnection* connection)
{
    if(connection != nullptr)
    {
        const GPtrArray* devices = nm_active_connection_get_devices(connection);
        for(int i = 0; i < devices->len; i++)
        {
            if(devices->pdata[i] == nmDevice)
            {
                return true;
            }
        }
    }
    return false;
}

    
/**
 * Scans for all visible NMAccessPoints and build the data structure 
 * mapping WifiAccessPoint objects to their NMAccessPoints.
 */
void LibNMHandler::buildAPMap()
{
    if(isWifiAvailable())
    {
        GLibSignalHandler signalHandler;
        signalHandler.gLibCall([this]()
        {
            const GPtrArray* visibleAPs = nm_device_wifi_get_access_points
                    (nmWifiDevice);
            const GPtrArray* wifiConns = nm_device_get_available_connections
                    (nmDevice);
            
            for(int apNum = 0; apNum < visibleAPs->len; apNum++)
            {
                NMAccessPoint* nmAP = (NMAccessPoint*) visibleAPs->pdata[apNum];
                NMConnection* apSavedConn = nullptr;
                GSList* wifiConnList = nullptr;
                for(int i = 0; i < wifiConns->len; i++)
                {
                    wifiConnList = g_slist_prepend(wifiConnList,
                            wifiConns->pdata[i]);
                }
                GSList* matchingConns = nm_access_point_filter_connections
                        (nmAP, wifiConnList);
                g_slist_free(wifiConnList);
                for(GSList* iter = matchingConns; iter != nullptr; 
                    iter = iter->next)
                {
                    NMConnection* conn 
                            = (NMConnection*) iter->data;
                    GError * error = nullptr;
                    if(nm_connection_verify(conn,&error))
                    {
                        apSavedConn = conn;
                        break;
                    }
                    else if(error != nullptr)
                    {
                        DBG("LibNMHandler: invalid connection: " 
                                << String(error->message));
                        g_error_free(error);
                    }
                }
                g_slist_free(matchingConns);
                WifiAccessPoint wifiAP(nmAP, apSavedConn);
                accessPointMap[wifiAP].addIfNotAlreadyThere(nmAP);
            }
        });
    }
}

/**
 * Connects a signal handler to the network manager client.
 */
gulong LibNMHandler::nmClientSignalConnect(
        const char* signal,
        GCallback signalHandler,
        gpointer callbackData)
{
    if (nmClient == nullptr)
    {
        return 0;
    }
    gulong handlerId = g_signal_connect
            (nmClient, signal, signalHandler, callbackData);
    clientSignalHandlers.add(handlerId);
    return handlerId;
}

/**
 * Connects a signal handler to the generic wlan0 device.
 */
gulong LibNMHandler::nmDeviceSignalConnect(
        const char* signal,
        GCallback signalHandler,
        gpointer callbackData)
{
    if (nmDevice == nullptr)
    {
        return 0;
    }
    gulong handlerId = g_signal_connect
            (nmDevice, signal, signalHandler, callbackData);
    deviceSignalHandlers.add(handlerId);
    return handlerId;
}

/**
 * Connects a signal handler to the wifi device wlan0.
 */
gulong LibNMHandler::nmWifiDeviceSignalConnect(
        const char* signal,
        GCallback signalHandler,
        gpointer callbackData)
{
    if (nmWifiDevice == nullptr)
    {
        return 0;
    }
    gulong handlerId = g_signal_connect
            (nmWifiDevice, signal, signalHandler, callbackData);
    wifiDeviceSignalHandlers.add(handlerId);
    return handlerId;
}

/**
 * Disconnects a signal handler from the network manager client.
 */
void LibNMHandler::nmClientSignalDisconnect(gulong toDisconnect)
{
    if (nmClient != nullptr)
    {
        g_signal_handler_disconnect(nmClient, toDisconnect);
        clientSignalHandlers.removeAllInstancesOf(toDisconnect);
    }
}

/**
 * Disconnects a signal handler from the generic wlan0 device
 */
void LibNMHandler::nmDeviceSignalDisconnect(gulong toDisconnect)
{
    if (nmDevice != nullptr)
    {
        g_signal_handler_disconnect(nmDevice, toDisconnect);
        deviceSignalHandlers.removeAllInstancesOf(toDisconnect);
    }
}

/**
 * Disconnects a signal handler from the wifi device wlan0
 */
void LibNMHandler::nmWifiSignalDisconnect(gulong toDisconnect)
{
    if (nmWifiDevice != nullptr)
    {
        g_signal_handler_disconnect(nmWifiDevice, toDisconnect);
        wifiDeviceSignalHandlers.removeAllInstancesOf(toDisconnect);
    }
}