#pragma once
/**
 * @file LibNM/AccessPoint.h
 * 
 * @brief A C++ interface for NMAccessPoint objects owned by LibNM.
 */

#include "GLib_Borrowed_Object.h"
#include "GLib_SignalHandler.h"
#include <nm-access-point.h>

namespace LibNM { class AccessPoint; }
namespace LibNM { class APHash; }
namespace LibNM { class SSID; }
namespace LibNM { enum class APMode; }
namespace LibNM { enum class SecurityType; }

/**
 *  AccessPoint holds a LibNM object representing a wifi access point.  It
 * provides access to relevant access point data, and checks Connection 
 * objects for compatibility with this access point.
 * 
 *  AccessPoint also provides the AccessPoint::Listener class as an interface 
 * for receiving access point signals. After they have been added, listeners 
 * will receive updates if signal strength changes or the access point is 
 * removed. 
 */

class LibNM::AccessPoint : public GLib::Borrowed::Object
{
public:
    /**
     * @brief  Creates a null AccessPoint.
     */
    AccessPoint();

    /**
     * @brief  Gets the access point's basic security type.
     *
     * @return  The appropriate access point security type, or 
     *          SecurityType::unsecured if the access point is null.
     */
    SecurityType getSecurityType() const;

    /**
     * @brief  Gets a hash value that may be used to identify and compare 
     *         access points, treating access points with shared connections as
     *         equivalent.
     *
     * @return  The access point hash value.  This will not be valid if the
     *          AccessPoint is null.
     */
    APHash generateHash() const;
    
    /**
     * @brief  Gets the access point SSID as a byte array from the access point.  
     *
     * This may contain unprintable characters, and might not be 
     * null-terminated.
     * 
     * @return  The exact SSID value, packaged in a byte array. 
     */
    const GByteArray* getSSID() const;
    
    /**
     * @brief  Gets the access point SSID, converted into a printable string 
     *         object.

     *  This value should only be used for displaying the access point name to
     * the user, or for debug output.
     * 
     * @return  The SSID as printable text.
     */
    juce::String getSSIDText() const;
    
    /**
     * @brief  Gets the BSSID of the wifi access point. In most cases, this will
     *         be the MAC address of the AP.
     * 
     * @return  The access point's BSSID, or the empty string if the access 
     *          point is null.
     */
    const char* getBSSID() const;
    
    /**
     * @brief  Gets the signal strength of the wifi access point.
     * 
     * @return  The wifi signal strength as an integer between 0 and 100, or 
     *          zero if the access point is null.
     */
    unsigned int getSignalStrength() const;
    
    /**
     * @brief  Gets the device mode of this access point.
     * 
     * @return  The access point's mode, or NM_802_11_MODE_UNKNOWN if the access
     *          point is null. 
     */
    NM80211Mode getMode() const;
   
    /**
     * @brief  Gets access point flags for this access point.
     * 
     * @return  Basic Wifi security flags for this access point, or 
     *          NM_802_11_AP_FLAGS_NONE if the access point is null.
     */
    NM80211ApFlags getFlags() const;
    
    /**
     * @brief  Gets WPA security flags for this access point.
     * 
     * @return  All WPA security flags describing the access point, or
     *          NM_802_11_AP_SEC_NONE if the access point is null. 
     */
    NM80211ApSecurityFlags getWPAFlags() const;
    
    /**
     * @brief  Gets RSN security flags for this access point.
     * 
     * @return  All RSN security flags describing the access point, or
     *          NM_802_11_AP_SEC_NONE if the access point is null. 
     */
    NM80211ApSecurityFlags getRSNFlags() const;
    
    /**
     * @brief  Gets the AccessPoint object's DBus path.
     *
     * @return  The DBus path to the objects's underlying NetworkManager DBus
     *          object, or the empty string if the object is null.
     */
    const char* getPath() const;
    
    /**
     * @brief  Listener objects can subscribe to receive updates when the access 
     *         point signal strength changes.
     */
    class Listener : public GLib::SignalHandler
    {
    public:     
        Listener() { }
        
        virtual ~Listener() { }
        
    private:   
        friend class AccessPoint;
        
        /**
         * @brief  Subscribes to signal strength signals from a single 
         *         NMAccessPoint.
         * 
         * @param source  A NMAccessPoint accessed as a GObject pointer.
         */
        virtual void connectAllSignals(GObject* source) final override;
        
        /**
         * @brief  Signals that the access point's signal strength has been 
         *         updated.
         * 
         * @param updatedAP    The updated wifi access point.
         * 
         * @param newStrength  The access point's new signal strength, ranging 
         *                     from zero to 100.
         */
        virtual void signalStrengthChanged(AccessPoint& updatedAP,
                unsigned int newStrength) = 0;
        
        /**
         * @brief  Builds signalStrengthChanged() calls from generic property 
         *         change notifications.
         * 
         * @param source    The NMAccessPoint responsible for the property 
         *                  change signal.
         * 
         * @param property  The updated property type.
         */
        virtual void propertyChanged
        (GObject* source, juce::String property) override;
    };
    
    /**
     * @brief  Adds a new listener to receive updates from this access point.
     * 
     * @param listener  An object that will be notified when connection signal
     *                  strength changes.
     */
    void addListener(Listener& listener);

private:
    /* Only the DeviceWifi object may create AccessPoints from raw 
       NMAccessPoint* data. */
    friend class DeviceWifi;

    /**
     * @brief  Creates a AccessPoint to contain a NMAccessPoint object.
     * 
     * @toAssign  A NMAccessPoint* for this AccessPoint to hold.
     */
    AccessPoint(NMAccessPoint* toAssign);

    /**
     * @brief  Gets the AccessPoint object's stored LibNM access point data.
     *
     * @return  The stored object data, cast to the LibNM access point type.
     */
    NMAccessPoint* getNMObjectPtr() const;
};
