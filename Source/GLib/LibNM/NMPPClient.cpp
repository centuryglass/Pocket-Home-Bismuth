#include "NMPPClient.h"

/*
 * Create a NMPPClient holding a new NMClient object.
 */
NMPPClient::NMPPClient() 
{ 
    callInMainContext([this]()
    {
        setGObject(G_OBJECT(nm_client_new()), false);
    });
}

/*
 * Create a NMPPClient that shares a NMClient with another NMPPClient.
 */
NMPPClient::NMPPClient(const NMPPClient& toCopy)
{ 
    setGObject(toCopy);
}

/*
 * Get all wifi devices from Network Manager.
 */
Array<NMPPDeviceWifi> NMPPClient::getWifiDevices() 
{ 
    Array<NMPPDeviceWifi> devices;
    callInMainContext([&devices](GObject* clientObject)
    {
        NMClient* client = NM_CLIENT(clientObject);
        if(client != nullptr)
        {
            const GPtrArray* devArray = nm_client_get_devices(client);
            for(int i = 0; devArray && i < devArray->len; i++)
            {
                NMDevice* dev = NM_DEVICE(devArray->pdata[i]);
                if(NM_IS_DEVICE_WIFI(dev))
                {
                    devices.add(NMPPDeviceWifi(NM_DEVICE_WIFI(dev)));
                }
            }  
        }
    });
    return devices;
}

/*
 * Gets a specific wifi device using its interface name.
 */
NMPPDeviceWifi NMPPClient::getWifiDeviceByIface(const char* interface) 
{ 
    NMPPDeviceWifi wifiDevice;
    callInMainContext([interface, &wifiDevice](GObject* clientObject)
    {
        NMClient* client = NM_CLIENT(clientObject);
        if(client != nullptr)
        {
            NMDevice* dev = nm_client_get_device_by_iface(client, interface);
            if(dev != nullptr && NM_IS_DEVICE_WIFI(dev))
            {
                wifiDevice = NM_DEVICE_WIFI(dev);
            }
        }
    });
    return wifiDevice;
}

/*
 * Gets a specific wifi device using its DBus path.
 */
NMPPDeviceWifi NMPPClient::getWifiDeviceByPath(const char* path) 
{ 
    NMPPDeviceWifi wifiDevice;
    callInMainContext([path, &wifiDevice](GObject* clientObject)
    {
        NMClient* client = NM_CLIENT(clientObject);
        if(client != nullptr)
        {
            NMDevice* dev = nm_client_get_device_by_path(client, path);
            if(dev != nullptr && NM_IS_DEVICE_WIFI(dev))
            {
                wifiDevice = NM_DEVICE_WIFI(dev);
            }
        }
    });
    return wifiDevice;
}

/*
 * Gets the list of all active connections known to the network manager.
 */
Array<NMPPConnection> NMPPClient::getActiveConnections() 
{ 
    Array<NMPPConnection> connections;
    callInMainContext([&connections](GObject* clientObject)
    {
        NMClient* client = NM_CLIENT(clientObject);
        if(client != nullptr)
        {
            const GPtrArray* cons = nm_client_get_active_connections(client);
            for(int i = 0; cons && i < cons->len; i++)
            {
                NMConnection* con = NM_CONNECTION(cons->pdata[i]);
                if(NM_IS_CONNECTION(con))
                {
                    connections.add(NMPPConnection(con));
                }
            }
        }
    });
    return connections;
}

/*
 * Gets the primary active network connection.
 */
NMPPConnection NMPPClient::getPrimaryConnection() 
{ 
    NMPPConnection primary;
    callInMainContext([&primary](GObject* clientObject)
    {
        NMClient* client = NM_CLIENT(clientObject);
        if(client != nullptr)
        {
            NMActiveConnection* con = nm_client_get_primary_connection(client);
            if(con != nullptr && NM_IS_CONNECTION(con))
            {
                primary = NM_CONNECTION(con);
            }
        }
    });
    return primary;
}

/*
 * Gets the connection being activated by the network manager.
 */
NMPPConnection NMPPClient::getActivatingConnection() 
{ 
    NMPPConnection activating;
    callInMainContext([&activating](GObject* clientObject)
    {
        NMClient* client = NM_CLIENT(clientObject);
        if(client != nullptr)
        {
            NMActiveConnection* con = nm_client_get_activating_connection(
                    client);
            if(con != nullptr && NM_IS_CONNECTION(con))
            {
                activating = NM_CONNECTION(con);
            }
        }
    });
    return activating;
}

/*
 * Deactivates an active network connection.
 */
void NMPPClient::deactivateConnection(const NMPPConnection& activeCon) 
{ 
    callInMainContext([this, &activeCon](GObject* clientObject)
    {
        NMClient* client = NM_CLIENT(clientObject);
        if(client != nullptr)
        {
            GObject* conObject = getOtherGObject(activeCon);
            if(conObject != nullptr)
            {
                if(NM_IS_ACTIVE_CONNECTION(conObject))
                {
                    nm_client_deactivate_connection(client,
                            NM_ACTIVE_CONNECTION(conObject));
                }
                g_object_unref(conObject);
            }         
        }
    });
}

/*
 * Checks if wireless connections are currently enabled.
 */
bool NMPPClient::wirelessEnabled() 
{ 
    bool enabled = false;
    callInMainContext([&enabled](GObject* clientObject)
    {
        NMClient* client = NM_CLIENT(clientObject);
        if(client != nullptr)
        {
            enabled = nm_client_wireless_get_enabled(client);
        }
    });
    return enabled;
}

/*
 * Sets whether wireless connections are enabled.
 */
void NMPPClient::setWirelessEnabled(bool enabled) 
{ 
    callInMainContext([enabled](GObject* clientObject)
    {
        NMClient* client = NM_CLIENT(clientObject);
        if(client != nullptr)
        {
            nm_client_wireless_set_enabled(client, enabled);
        }
    });
}


/*
 * The NMClientActivateFn called by LibNM when activating an existing
 * connection, used to create a call to openingConnection or to 
 * openingConnectionFailed.
 */
void NMPPClient::ConnectionHandler::activateCallback(NMClient* client,
        NMActiveConnection* connection,
        GError* error,
        NMPPClient::ConnectionHandler* handler) 
{
    if(error != nullptr)
    {
        handler->openingConnectionFailed
                (NMPPConnection(NM_CONNECTION(connection)), error, false);
        g_error_free(error);
    }
    else
    {
        handler->openingConnection    
                (NMPPConnection(NM_CONNECTION(connection)), false);
    } 
}

/*
 * The NMClientAddActivateFn called by LibNM when adding and activating
 * a new connection, used to create a call to openingConnection or 
 * openingConnectionFailed.
 */
void NMPPClient::ConnectionHandler::addActivateCallback(NMClient* client,
        NMConnection* connection,
        const char* path,
        GError* error,
        NMPPClient::ConnectionHandler* handler) 
{
    if(error != nullptr)
    {
        handler->openingConnectionFailed
                (NMPPConnection(NM_CONNECTION(connection)), error, true);
        g_error_free(error);
    }
    else
    {
        handler->openingConnection    
                (NMPPConnection(NM_CONNECTION(connection)), true);
    } 
}


/*
 * Activates a wifi network connection, attempting to set it as the primary
 * network connection.
 */ 
void NMPPClient::activateConnection(
        const NMPPConnection& connection,
        const NMPPDeviceWifi& wifiDevice,
        const NMPPAccessPoint& accessPoint, 
        NMPPClient::ConnectionHandler* handler)
{ 
    
    //determine if this is a new connection attempt
    bool isNew = false;
    Array<NMPPConnection> connections = wifiDevice.getAvailableConnections();
    for(const NMPPConnection& saved : connections)
    {
        if(saved.connectionMatches(connection))
        {
            isNew = true;
            break;
        }
    }
    callInMainContext(
            [this, isNew, &connection, &wifiDevice, &accessPoint, handler]
            (GObject* clientObject)
    {
        NMClient* client = NM_CLIENT(clientObject);
        if(client != nullptr)
        {
            const char* apPath = accessPoint.getPath();
            GObject* conObj = getOtherGObject(connection);
            GObject* devObj = getOtherGObject(wifiDevice);
            
            //connecting with a null connection object is allowed and may work.
            if(apPath != nullptr && devObj != nullptr)
            {
                if(isNew)
                {
                    nm_client_add_and_activate_connection(client,
                        NM_CONNECTION(conObj),
                        NM_DEVICE(devObj),
                        apPath,
                        (NMClientAddActivateFn) 
                        ConnectionHandler::addActivateCallback,
                        handler);   
                }
                else
                {
                    nm_client_activate_connection(client,
                            NM_CONNECTION(conObj),
                            NM_DEVICE(devObj),
                            apPath,
                            (NMClientActivateFn) 
                            ConnectionHandler::activateCallback,
                            handler);  
                }
            }
            if(conObj != nullptr)
            {
                g_object_unref(conObj);
            }
            if(devObj != nullptr)
            {
                g_object_unref(devObj);
            }
        }
    });
}

/*
 * Converts generic propertyChanged calls to class-specific 
 * wirelessStateChange calls.
 */
void NMPPClient::Listener::propertyChanged(GPPObject* source, String property)
{ 
    NMPPClient* client = dynamic_cast<NMPPClient*>(source);
    if(client != nullptr && property == NM_CLIENT_WIRELESS_ENABLED)
    {
        bool enabled = client->wirelessEnabled();
        wirelessStateChange(enabled);
    }
}

/*
 * Adds a listener to this network manager client.
 */
void NMPPClient::addListener(NMPPClient::Listener* listener)
{ 
    addNotifySignalHandler(listener, NM_CLIENT_WIRELESS_ENABLED);
}

/*
 * Removes a listener from this network manager client.
 */
void NMPPClient::removeListener(NMPPClient::Listener* listener)
{ 
    removeSignalHandler(listener);
}

/*
 * Gets the GType of this object's stored GObject class.
 */
GType NMPPClient::getType() const 
{
    return NM_TYPE_CLIENT;
}

/*
 * Checks if a GObject's type allows it to be held by this object. 
 */
bool NMPPClient::isValidType(GObject* toCheck) const 
{ 
    return NM_IS_CLIENT(toCheck);
}

/*
 * Used to re-add a list of Listeners to new GObject data.
 */
void NMPPClient::transferSignalHandlers
(Array<GPPObject::SignalHandler*>& toTransfer)
{
    for(SignalHandler* handler : toTransfer)
    {
        addListener(static_cast<Listener*>(handler));
    }
}