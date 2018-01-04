/*
  ==============================================================================

    WifiIcon.h
    Created: 29 Dec 2017 12:02:41pm
    Author:  anthony

  ==============================================================================
 */

#pragma once
#include "../../Basic Components/VectorImageButton.h"

/**
 * WifiIcon tracks WiFi status and displays an image indicating connection
 * state.
 */
class WifiIcon : public VectorImageButton{
public:
    WifiIcon();
    virtual ~WifiIcon();

private:
    //All tracked WiFi states.  Each corresponds with an image asset file
    //defined in config.json
    enum WifiIconImage {
        WIFI_OFF,
        WIFI_STRENGTH_0,
        WIFI_STRENGTH_1,
        WIFI_STRENGTH_2,
        WIFI_STRENGTH_3
    };
    /**
     * Set the WiFi connection status image.
     * @param wifiState the last discovered state of the WiFi connection.
     */
    void setStatus(WifiIconImage wifiState);
    /**
     * Enable/disable the WiFi checking timer based on component visibility
     */
    void visibilityChanged() override;
    /**
     * WifiTimer periodically checks the current WiFi connection state, and
     * updates the WiFi icon.
     */
    class WifiTimer : public Timer {
    public:
        WifiTimer(WifiIcon * wifiIcon);
        virtual ~WifiTimer();
        void removeTimer();
    private:
        void timerCallback();
        WifiIcon * wifiIcon;
        const static int frequency = 2000;
    };
    ScopedPointer<WifiTimer> wifiTimer;

};