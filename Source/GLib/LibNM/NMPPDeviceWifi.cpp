#include "NMPPDeviceWifi.h"

/*
 * Creates a NMPPDeviceWifi object that shares a NMDeviceWifi*
 * with another NMPPDeviceWifi object
 */
NMPPDeviceWifi::NMPPDeviceWifi(const NMPPDeviceWifi& toCopy)
{ 
    setGObject(toCopy);
}

/*
 * Create a NMPPDeviceWifi to contain a NMDeviceWifi object.
 */
NMPPDeviceWifi::NMPPDeviceWifi(NMDeviceWifi* toAssign)
{
    setGObject(G_OBJECT(toAssign));
}

/*
 * Gets the current state of the wifi network device.
 */
NMDeviceState NMPPDeviceWifi::getState() const
{ 
    NMDeviceState state = NM_DEVICE_STATE_UNKNOWN;
    callInMainContext([&state](GObject* devObject)
    {
        NMDevice* device = NM_DEVICE(devObject);
        if(device != nullptr)
        {
            state = nm_device_get_state(device);
        }
    });
    return state;
}

/*
 * Gets the reason for the current device state.
 */
NMDeviceStateReason NMPPDeviceWifi::getStateReason() const
{ 
    NMDeviceStateReason reason = NM_DEVICE_STATE_REASON_UNKNOWN;
    callInMainContext([&reason](GObject* devObject)
    {
        NMDevice* device = NM_DEVICE(devObject);
        if(device != nullptr)
        {
            nm_device_get_state_reason(device, &reason);
        }
    });
    return reason;
}

/*
 * Checks if NetworkManager manages this device.
 */
bool NMPPDeviceWifi::isManaged() const
{ 
    bool managed = false;
    callInMainContext([&managed](GObject* devObject)
    {
        NMDevice* device = NM_DEVICE(devObject);
        if(device != nullptr)
        {
            managed = nm_device_get_managed(device);
        }
    });
    return managed;
}

/*
 * Gets the interface name of this wifi device.
 */
const char* NMPPDeviceWifi::getInterface() const
{ 
    const char* iface = nullptr;
    callInMainContext([&iface](GObject* devObject)
    {
        NMDevice* device = NM_DEVICE(devObject);
        if(device != nullptr)
        {
            iface = nm_device_get_iface(device);
        }
    });
    return iface;
}

/*
 * Gets the DBus path of this wifi device.
 */
const char* NMPPDeviceWifi::getPath() const
{ 
    const char* path = nullptr;
    callInMainContext([&path](GObject* devObject)
    {
        NMObject* device = NM_OBJECT(devObject);
        if(device != nullptr)
        {
            path = nm_object_get_path(device);
        }
    });
    return path;
}

/*
 * Disconnects any connection that is active on this wifi device.  If there
 * is no active connection, or the object is null, nothing will happen.
 */
void NMPPDeviceWifi::disconnect() 
{ 
    callInMainContext([](GObject* devObject)
    {
        NMDevice* device = NM_DEVICE(devObject);
        if(device != nullptr)
        {
            nm_device_disconnect(device, nullptr, nullptr);
        }
    });
}

/*
 * Gets the current active connection running on this device.
 */
NMPPConnection NMPPDeviceWifi::getActiveConnection() const
{ 
    NMPPConnection active;
    callInMainContext([&active](GObject* devObject)
    {
        NMDevice* device = NM_DEVICE(devObject);
        if(device != nullptr)
        {
            NMActiveConnection* con = nm_device_get_active_connection(device);
            if(con != nullptr)
            {
                active = NM_CONNECTION(con);
            }
        }
    });
    return active;
}
  
/*
 * Gets the list of connections available to activate on this device.
 * This might not load all saved connections.
 */
Array<NMPPConnection> NMPPDeviceWifi::getAvailableConnections() const
{ 
    Array<NMPPConnection> available;
    callInMainContext([&available](GObject* devObject)
    {
        NMDevice* device = NM_DEVICE(devObject);
        if(device != nullptr)
        {
            const GPtrArray* cons = nm_device_get_available_connections(device);
            for(int i = 0; cons && i < cons->len; i++)
            {
                NMConnection* connection = NM_CONNECTION(cons->pdata[i]);
                if(connection != nullptr)
                {
                    available.add(NMPPConnection(connection));
                }
            }
        }
    });
    return available;
}

/*
 * Gets an access point object using the access point's path.
 */
NMPPAccessPoint NMPPDeviceWifi::getAccessPoint(const char* path) const
{ 
    NMPPAccessPoint ap;
    callInMainContext([&ap, path](GObject* devObject)
    {
        NMDeviceWifi* device = NM_DEVICE_WIFI(devObject);
        if(device != nullptr)
        {
            ap = nm_device_wifi_get_access_point_by_path(device, path);
        }
    });
    return ap;
}

/*
 * Gets all access points visible to this device.
 */
Array<NMPPAccessPoint> NMPPDeviceWifi::getAccessPoints() const
{ 
    Array<NMPPAccessPoint> accessPoints;
    callInMainContext([&accessPoints](GObject* devObject)
    {
        NMDeviceWifi* device = NM_DEVICE_WIFI(devObject);
        if(device != nullptr)
        {
            const GPtrArray* aps = nm_device_wifi_get_access_points(device);
            for(int i = 0; aps && i < aps->len; i++)
            {
                NMAccessPoint* nmAP = NM_ACCESS_POINT(aps->pdata[i]);
                if(nmAP != nullptr)
                {
                    accessPoints.add(NMPPAccessPoint(nmAP));
                }
            }
        }
    });
    return accessPoints;
}

/*
 * Sends a request to the wifi device asking it to scan visible access 
 * points.
 */
void NMPPDeviceWifi::requestScan() 
{ 
    callInMainContext([](GObject* devObject)
    {
        NMDeviceWifi* device = NM_DEVICE_WIFI(devObject);
        if(device != nullptr)
        {
            nm_device_wifi_request_scan_simple(device, nullptr, nullptr);
        }
    });
}

/*
 * Adds a listener object to this wifi device.
 */
void NMPPDeviceWifi::addListener(Listener* listener) 
{ 
    addSignalHandler(listener, "state-changed",
            G_CALLBACK(stateChangeCallback));
    addSignalHandler(listener, "access-point-added", 
            G_CALLBACK(apAddedCallback));
    addSignalHandler(listener, "access-point-removed",
            G_CALLBACK(apRemovedCallback));
}

/*
 * Removes a listener object from this wifi device.
 */
void NMPPDeviceWifi::removeListener(Listener* listener) 
{ 
    removeSignalHandler(listener);
}

/*
 * The GCallback method called directly by LibNM code when sending 
 * state-changed signals.  This will use the signal data to call the 
 * listener object's stateChanged method.
 */
void NMPPDeviceWifi::stateChangeCallback(
        NMDevice* device,
        NMDeviceState newState,
        NMDeviceState oldState,
        NMDeviceStateReason reason,
        Listener* listener) 
{ 
    NMPPDeviceWifi* deviceWrapper = dynamic_cast<NMPPDeviceWifi*>
            (GPPObject::findObjectWrapper(G_OBJECT(device), listener));
    listener->stateChanged(deviceWrapper, newState, oldState, reason);
}

/*
 * The GCallback method called directly by LibNM code when sending 
 * access-point-added signals.  This will use the signal data to call the
 * listener object's accessPointAdded method
 */
void NMPPDeviceWifi::apAddedCallback(NMDeviceWifi* device, NMAccessPoint* ap,
        Listener* listener) 
{ 
    listener->accessPointAdded(NMPPAccessPoint(ap));
}

/*
 * The GCallback method called directly by LibNM code when sending 
 * access-point-removed signals.  This will use the signal data to call the
 * listener object's accessPointRemoved method
 */
void NMPPDeviceWifi::apRemovedCallback(NMDeviceWifi* device, NMAccessPoint* ap,
        Listener* listener) 
{ 
    listener->accessPointRemoved(NMPPAccessPoint(ap));
}

/*
 * Gets the GType of this object's stored GObject class.
 */
GType NMPPDeviceWifi::getType() const 
{ 
    return NM_TYPE_DEVICE_WIFI;
}

/*
 * Checks if a GObject's type allows it to be held by this object. 
 */
bool NMPPDeviceWifi::isValidType(GObject* toCheck) const 
{ 
    return NM_IS_DEVICE_WIFI_CLASS(toCheck);
}

/*
 * Used to re-add a list of Listeners to new GObject data.
 */
void NMPPDeviceWifi::transferSignalHandlers(Array<SignalHandler*>& toTransfer)
{
    for(SignalHandler* handler : toTransfer)
    {
        addListener(static_cast<Listener*>(handler));
    }
}