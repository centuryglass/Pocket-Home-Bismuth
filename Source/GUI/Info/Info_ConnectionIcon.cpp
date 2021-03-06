#include "Info_ConnectionIcon.h"
#include "Wifi_AccessPoint.h"
#include "Wifi_Device_Reader.h"
#include "Wifi_Connection_Record_Handler.h"
#include "Util_SafeCall.h"

#ifdef JUCE_DEBUG
// Print the full class name before all debug output:
static const constexpr char* dbgPrefix = "Info::ConnectionIcon::";
#endif

Info::ConnectionIcon::ConnectionIcon()
{
#if JUCE_DEBUG
    setName("Info::ConnectionIcon");
#endif
    const Wifi::Connection::Record::Handler connectionRecords;
    if (connectionRecords.isConnected())
    {
        Wifi::AccessPoint connectedAP = connectionRecords.getActiveAP();
        setTrackedAccessPoint(connectedAP);
        setSignalStrengthImage(connectedAP.getSignalStrength());
        DBG(dbgPrefix << __func__ << ": Initializing with connected AP "
                << connectedAP.getSSID().toString());
        return;
    }

    const Wifi::Device::Reader deviceReader;
    if (deviceReader.wifiDeviceEnabled())
    {
        DBG(dbgPrefix << __func__ << ": Initializing with wifi disconnected.");
        setSignalStrengthImage(0);
    }
    else
    {
        DBG(dbgPrefix << __func__ << ": Initializing with wifi disabled.");
        setDisabledImage();
    }
}


// Updates the selected icon when the active connection's signal strength
// changes.
void Info::ConnectionIcon::signalStrengthUpdate
(const Wifi::AccessPoint updatedAP)
{
    setSignalStrengthImage(updatedAP.getSignalStrength());
}


// Changes the tracked AccessPoint and selected status icon when a new Wifi
// connection is activated.
void Info::ConnectionIcon::connected(const Wifi::AccessPoint connectedAP)
{
    setTrackedAccessPoint(connectedAP);
    setSignalStrengthImage(connectedAP.getSignalStrength());
    DBG(dbgPrefix << __func__ << ": Now tracking connected AP "
            << connectedAP.getSSID().toString());
}


// Stops tracking signal strength changes and updates the status icon when the
// Wifi connection is lost.
void Info::ConnectionIcon::disconnected(const Wifi::AccessPoint disconnectedAP)
{
    const Wifi::Device::Reader deviceReader;
    if (deviceReader.wifiDeviceEnabled())
    {
        DBG(dbgPrefix << __func__
                << ": Wifi disconnected, setting image to signal strength 0");
        ignoreAllUpdates();
        setSignalStrengthImage(0);
    }
}


// Updates the selected icon when Wifi is turned on.
void Info::ConnectionIcon::wirelessEnabled()
{
    DBG(dbgPrefix << __func__ << ": Wifi connected, updating signal image");
    Wifi::Connection::Record::Handler connectionRecord;
    if (connectionRecord.isConnected())
    {
        connected(connectionRecord.getActiveAP());
    }
    else
    {
        setSignalStrengthImage(0);
    }
}


// Updates the selected icon and stops tracking signal strength when Wifi is
// turned off.
void Info::ConnectionIcon::wirelessDisabled()
{
    DBG(dbgPrefix << __func__ << ": Wifi disabled, setting disabled image");
    ignoreAllUpdates();
    Util::SafeCall::callAsync<ConnectionIcon>(this,[](ConnectionIcon* icon)
    {
        icon->setDisabledImage();
    });
}


// Safely updates the connection icon asynchronously on the message thread.
void Info::ConnectionIcon::asyncSetSignalImage
(const unsigned int signalStrength)
{
    Util::SafeCall::callAsync<ConnectionIcon>(this,
    [signalStrength](ConnectionIcon* icon)
    {
        icon->setSignalStrengthImage(signalStrength);
    });
}
