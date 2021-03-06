#include "Page_Power.h"
#include "Page_Type.h"
#include "Layout_Transition_Animator.h"
#include "Util_Commands.h"
#include "PocketHomeWindow.h"
#include <map>

// Localized object class key:
static const juce::Identifier localeClassKey = "Page::Power";

// Localized text keys:
namespace TextKey
{
    static const juce::Identifier shutdown      = "shutdown";
    static const juce::Identifier reboot        = "reboot";
    static const juce::Identifier sleep         = "sleep";
    static const juce::Identifier build         = "build";
    static const juce::Identifier version       = "version";
#ifdef CHIP_FEATURES
    static const juce::Identifier flashSoftware = "flashSoftware";
#endif
}


// Page layout constants:
static const constexpr int labelRowWeight   = 6;
static const constexpr int buttonRowWeight  = 10;
static const constexpr int rowPaddingWeight = 4;

static const constexpr float yMarginFraction = 0.1;

Page::Power::Power() : Locale::TextUser(localeClassKey),
pageListener(*this),
powerOffButton(localeText(TextKey::shutdown)),
rebootButton(localeText(TextKey::reboot)),
sleepButton(localeText(TextKey::sleep)),
#ifdef CHIP_FEATURES
felButton(localeText(TextKey::flashSoftware)),
#endif
versionLabel(localeText(TextKey::version)
        + juce::JUCEApplication::getInstance()->getApplicationVersion())
{
#ifdef JUCE_DEBUG
    setName("Page::Power");
#endif
    using namespace Layout::Group;
    RelativeLayout layout({
        Row(labelRowWeight,
        {
            RowItem(&versionLabel)
        }),
        Row(buttonRowWeight,
        {
            RowItem(&powerOffButton)
        }),
        Row(buttonRowWeight,
        {
            RowItem(&sleepButton)
        }),
        Row(buttonRowWeight,
        {
            RowItem(&rebootButton)
        }),
#ifdef CHIP_FEATURES
        Row(buttonRowWeight,
        {
            RowItem(&felButton)
        }),
#endif
        Row(labelRowWeight,
        {
            RowItem(&buildLabel)
        })
    });
    layout.setYMarginFraction(yMarginFraction);
    layout.setYPaddingWeight(rowPaddingWeight);
    setBackButton(BackButtonType::right);
    setLayout(layout);

    setColour(Component::backgroundColourId, findColour(background));

    versionLabel.setJustificationType(juce::Justification::centred);
    versionLabel.setText(localeText(TextKey::version)
            + juce::JUCEApplication::getInstance()->getApplicationVersion(),
            juce::NotificationType::dontSendNotification);

    // Determine release label contents
    juce::String buildText = localeText(TextKey::build) + " ";
#ifdef BUILD_NAME
    buildText += juce::String(BUILD_NAME);
#endif
    buildLabel.setJustificationType(juce::Justification::centred);
    buildLabel.setText(buildText,
            juce::NotificationType::dontSendNotification);

    powerOffButton.addListener(&pageListener);
    sleepButton.addListener(&pageListener);
    rebootButton.addListener(&pageListener);
#ifdef CHIP_FEATURES
    felButton.addListener(&pageListener);
#endif
    addChildComponent(overlaySpinner);
    addAndShowLayoutComponents();
}


// Turns off the display until key or mouse input is detected.
void Page::Power::startSleepMode()
{
    Util::Commands systemCommands;
    if (systemCommands.runIntCommand(Util::CommandTypes::Int::sleepCheck) != 0)
    {
        PocketHomeWindow* window = PocketHomeWindow::getOpenWindow();
        jassert(window != nullptr);
        window->showLoginScreen();
        // Make sure power page buttons can't be accidentally clicked from
        // sleep mode:
        setEnabled(false);
        systemCommands.runActionCommand(Util::CommandTypes::Action::sleep);
    }
    else
    {
        systemCommands.runActionCommand(Util::CommandTypes::Action::wake);
    }
    // Enable the page again after waking. This shouldn't happen before the
    // login page is shown.
    setEnabled(true);
}


// Shows the power spinner to indicate to the user that the system is
// restarting or shutting down.
void Page::Power::showPowerSpinner()
{
    powerOffButton.setVisible(false);
    sleepButton.setVisible(false);
    rebootButton.setVisible(false);
#ifdef CHIP_FEATURES
    felButton.setVisible(false);
#endif
    overlaySpinner.setVisible(true);
}


// Resizes the lock screen and overlay spinner to fit the page.
void Page::Power::pageResized()
{
    overlaySpinner.setBounds(getLocalBounds());
}


// Handles all button clicks.
void Page::Power::PageListener::buttonClicked(juce::Button *button)
{
#ifdef CHIP_FEATURES
    if (button == &powerPage.felButton)
    {
        powerPage.pushPageToStack(Page::Type::fel,
                Layout::Transition::Type::moveRight);
        return;
    }
#endif
    if (button == &powerPage.sleepButton)
    {
        powerPage.startSleepMode();
        return;
    }
    Util::Commands systemCommands;
    if (button == &powerPage.powerOffButton)
    {
        powerPage.showPowerSpinner();
        systemCommands.runActionCommand(Util::CommandTypes::Action::shutdown);
    }
    else if (button == &powerPage.rebootButton)
    {
        powerPage.showPowerSpinner();
        systemCommands.runActionCommand(Util::CommandTypes::Action::restart);
    }
}
