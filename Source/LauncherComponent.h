#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "OverlaySpinner.h"
#include "LauncherBarComponent.h"
#include "PageStackComponent.h"
#include "BatteryMonitor.h"
#include "SwitchComponent.h"
#include "ClockMonitor.hpp"
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

class LauncherComponent;
class LibraryPageComponent;
class AppsPageComponent;

class BatteryIconTimer : public Timer {
public:
    BatteryIconTimer() {};
    void timerCallback();
    LauncherComponent* launcherComponent;
    OwnedArray<ImageButton> * buttons;
};

class WifiIconTimer : public Timer {
public:
  WifiIconTimer() {};
  void timerCallback();
  LauncherComponent* launcherComponent;
  OwnedArray<ImageButton> * buttons;
};

class LauncherComponent : public Component, private Button::Listener {
public:
    BatteryMonitor batteryMonitor;
    ScopedPointer<ImageComponent> focusButtonPopup;
  
    Array<Image> batteryIconImages;
    Array<Image> batteryIconChargingImages;
    Array<Image> wifiIconImages;
    
    ScopedPointer<Label> batteryLabel;
    ScopedPointer<Label> modeLabel;
  
    BatteryIconTimer batteryIconTimer;
    WifiIconTimer wifiIconTimer;
    Component* defaultPage;
  
    // FIXME: we have no need for the pages/pagesByName if we're using scoped pointers for each page.
    // All these variables do is add an extra string key the compiler can't see through.
    OwnedArray<Component> pages;
    ScopedPointer<PageStackComponent> pageStack;
    HashMap<String, Component *> pagesByName;
    
    bool resize = false;
    
    StretchableLayoutManager categoryButtonLayout;
    
    LauncherComponent();
    ~LauncherComponent();
    
    void paint(Graphics &) override;
    void resized() override;
    void updateIp();
    void setIpVisible(bool);
    
    void setClockAMPM(bool);
    void setColorBackground(const String&);
    void setImageBackground(const String&);
    void setClockVisible(bool);
    
    void hideLaunchSpinner();
    
private:
    Colour bgColor;
    Label labelip;
    Image bgImage;
    bool hasImg;
    ScopedPointer<ClockMonitor> clock;
    OwnedArray<ImageButton> cornerButtons;
  
    void buttonClicked(Button *) override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LauncherComponent)
};
