/**
 * @file AppLauncher.h
 * 
 * AppLauncher launches and tracks new application processes, automatically
 * switching window focus to the new application.
 */
#pragma once
#include <functional>
#include <map>
#include "Localized.h"
#include "JuceHeader.h"
#include "WindowFocusedTimer.h"

/**
 * TODO:
 *  -Test to see if it's really necessary to reload xmodmap every time an 
 *   application launches
 */

class AppLauncher : public WindowFocusedTimer, private Localized
{
public:
    AppLauncher() : WindowFocusedTimer("AppLauncher"),
    Localized("AppLauncher"),
    launchFailureCallback([]()
    {
    }) { }

    virtual ~AppLauncher() { }

    /**
     * Assigns a function to call if loading an application fails.
     * @param failureCallback will run if an app fails to launch.
     */
    void setLaunchFailureCallback(std::function<void() > failureCallback)
    {
        launchFailureCallback = failureCallback;
    }

    /**
     * Launch a new application, or focus its window if the application is
     * already running
     * @param appTitle the application title
     * @param command the shell command that starts the application
     */
    void startOrFocusApp(String appTitle, String command);
private:

    /**
     * Stores information on launched processes
     */
    struct ProcessInfo
    {
        ProcessInfo(String title, String command);
        //Application title
        String title;
        //Application launch command
        String command;
        bool operator==(const ProcessInfo& rhs) const;
        bool operator<(const ProcessInfo& rhs) const;
    };

    /**
     * Start a new instance of an application process
     * @param processInfo defines the new application title and
     * launch command.
     */
    void startApp(ProcessInfo processInfo);

    /**
     * Focus the window of a running app
     * @param windowId the app window's id
     */
    void focusApp(const String& windowId);

    /**
     * Attempt to find an open window of a launched application
     * @param processInfo running application process info
     * @return the window ID, or the empty string if none was found
     */
    String getWindowId(ProcessInfo processInfo);

    //Track application launch success and respond appropriately.
    virtual void timerCallback() override;

    /**
     * Stop process tracking if window focus changes and the timer is suspended.
     */
    virtual void onSuspend() override;

    //function to run if application launching fails
    std::function<void() > launchFailureCallback;

    //holds all running processes launched by this object.
    OwnedArray<ChildProcess> runningApps;

    //store child process launch information
    std::map<ProcessInfo, ChildProcess*> processMap;

    //timer interval in milliseconds
    static const int timerFrequency = 2000;

    //milliseconds to wait before giving up on a launch

    static const int timeout = 15000;

    //last launch time from Time::getMillisecondCounter()
    uint32 lastLaunch = 0;

    //Process to check up on when the timer finishes
    ChildProcess * timedProcess = nullptr;

    //localized text keys;
    static const constexpr char * could_not_open = "could_not_open";
    static const constexpr char * not_valid_command = "not_valid_command";


};
