#include "DateTimePage.h"

DateTimePage::DateTimePage(
        MainConfigFile& mainConfig,
        ComponentConfigFile& componentConfig) :
Localized("DateTimePage"),
PageComponent(componentConfig, "DateTimePage",{
    {3,
        {
            {&titleLabel, 1}
        }},
    {2,
        {
            {&clockModeLabel, 3},
            {&setClockMode, 1}
        }},
    {3,
        {
            {nullptr, 1}
        }},
    {2,
        {
            {&reconfigureBtn, 1}
        }},
    {1,
        {
            {nullptr, 1}
        }}
}),
mainConfig(mainConfig),
componentConfig(componentConfig),
titleLabel(componentConfig, "dateTimeTitle", localeText(date_time_settings)),
setClockMode("setClockMode"),
reconfigureBtn(localeText(set_system_clock)),
clockModeLabel(componentConfig, "modeLabel", localeText(select_clock_mode))
{

#    if JUCE_DEBUG
    setName("DateTimePage");
#    endif
    addAndShowLayoutComponents();
    reconfigureBtn.addListener(this);
    titleLabel.setJustificationType(Justification::centred);

    setClockMode.addItem(localeText(mode_24h), 1);
    setClockMode.addItem(localeText(mode_am_pm), 2);
    setClockMode.addItem(localeText(hide_clock), 3);
    setClockMode.addListener(this);
    if (componentConfig.getConfigValue<bool>(ComponentConfigFile::showClockKey))
    {
        if (componentConfig.getConfigValue<bool>
            (ComponentConfigFile::use24HrModeKey))
        {
            setClockMode.setSelectedId(1,
                    NotificationType::dontSendNotification);
        }
        else
        {
            setClockMode.setSelectedId(2,
                    NotificationType::dontSendNotification);
        }
    }
    else
    {
        setClockMode.setSelectedId(3, NotificationType::dontSendNotification);
    }
}

/**
 * Runs reconfigureCommand in the terminal to update system time when
 * the user presses the reconfigure button.
 */
void DateTimePage::pageButtonClicked(Button* button)
{
    if (button == &reconfigureBtn)
    {
        String configureTime = mainConfig.getConfigValue<String>
                (MainConfigFile::termLaunchCommandKey)
                + reconfigureCommand;
        int ret = system(configureTime.toRawUTF8());
        if (ret != 0)
        {
            AlertWindow::showMessageBox(AlertWindow::WarningIcon,
                    localeText(failed_launch),
                    localeText(failed_to_run)
                    + "\n" + configureTime + "\n"
                    + localeText(check_term_cmd));
        }
    }
}

/**
 * Changes the clock mode saved in the ComponentConfigFile when the
 * user selects a new mode with the setClockMode combo box.
 */
void DateTimePage::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox != &setClockMode)
    {
        DBG("DateTimePage::" << __func__ << ": responding to ComboBox "
                << comboBox->getName() << ", this should not happen!")
        return;
    }
    bool showClock = (comboBox->getSelectedId() != 3);
    bool use24HrMode = (comboBox->getSelectedId() == 1);
    if (showClock)
    {
        componentConfig.setConfigValue<bool>
                (ComponentConfigFile::use24HrModeKey, use24HrMode);
    }
    componentConfig.setConfigValue<bool>
            (ComponentConfigFile::showClockKey, showClock);
}

