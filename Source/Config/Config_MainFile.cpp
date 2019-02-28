#include "Config_MainFile.h"
#include "Config_MainKeys.h"
#include "Config_MainResource.h"

Config::MainFile::MainFile() { }

/*
 * Gets how frequently the Wifi settings page should scan for access point
 * updates.
 */
int Config::MainFile::getWifiScanFrequency() const
{
    return getConfigValue<int>(MainKeys::wifiScanFreqKey);
}

/*
 * Gets the prefix to place before system commands when they should be launched
 * within a new terminal window.
 */
juce::String Config::MainFile::getTermLaunchPrefix() const
{
    return getConfigValue<juce::String>(MainKeys::termLaunchCommandKey);
}

/*
 * Gets the Wifi interface to use when managing Wifi connections.
 */
juce::String Config::MainFile::getWifiInterface() const
{
    return getConfigValue<juce::String>(MainKeys::wifiInterfaceKey);
}

/*
 * Gets if the mouse cursor should be shown when pocket-home is active.
 */
bool Config::MainFile::getShowCursor() const
{
    return getConfigValue<bool>(MainKeys::showCursorKey);
}

/*
 * Gets if the clock will be shown within the HomePage component.
 */
bool Config::MainFile::getShowClock() const
{
    return getConfigValue<bool>(MainKeys::showClockKey);
}

/*
 * Gets if the clock label will show time in 24 hour mode, or in 12 hour AM/PM
 * mode.
 */
bool Config::MainFile::get24HourEnabled() const
{
    return getConfigValue<bool>(MainKeys::use24HrModeKey);
}

/*
 * Sets if the mouse cursor is shown or hidden.
 */
void Config::MainFile::setShowCursor(const bool showCursor)
{
    setConfigValue<bool>(MainKeys::showCursorKey, showCursor);
}

/*
 * Sets if the clock label is shown or hidden.
 */
void Config::MainFile::setShowClock(const bool showClock)
{
    setConfigValue<bool>(MainKeys::showClockKey, showClock);
}

/*
 * Sets if the clock label will use 24 hour mode.
 */
void Config::MainFile::set24HourEnabled(const bool use24HourMode)
{
    setConfigValue<bool>(MainKeys::use24HrModeKey, use24HourMode);
}
