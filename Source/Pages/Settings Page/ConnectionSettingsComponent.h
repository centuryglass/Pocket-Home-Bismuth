/**
 * @file ConnectionSettingsComponent.h
 * 
 * ConnectionSettingsComponent is a widget that provides a connection status
 * icon, a toggle switch for enabling/disabling the connection, and a button
 * that shows a connection status string, and opens a connection page when
 * clicked while the switch is enabled.
 */
#pragma once

#include "JuceHeader.h"
#include "PageComponent.h"
#include "RelativeLayoutManager.h"
#include "SwitchComponent.h"
#include "Spinner.h"
#include "DrawableImageComponent.h"

class ConnectionSettingsComponent : public Component,
private Button::Listener
{
public:
    /**
     * @param openConnectionPage callback to run when the button is clicked.
     * This should be a function that opens a connection settings page.
     * @param name internal component name.
     */
    ConnectionSettingsComponent(std::function<void() > openConnectionPage,
            const String& name = String());

    virtual ~ConnectionSettingsComponent() { }

protected:
    /**
     * Updates the icon, switch, and connection button based on current
     * connection status.
     */
    void refresh();

private:
    /**
     * Check the connections enabled status directly.  This will be used to
     * determine what position the switch should be in when the component
     * is created or refreshed.
     * @return true if connections are enabled, false if disabled.
     */
    virtual bool connectionEnabled() = 0;

    /**
     * This is used to determine when the component should show a loading
     * spinner and disable all input.
     * @return true if connection status is busy, false if it's ready for
     * input.
     */
    virtual bool isBusy() = 0;

    /**
     * @return an appropriate asset file name for the connection type and
     * status.
     */
    virtual String getIconAsset() = 0;

    /**
     * This will be called whenever the enable/disable switch is flipped.
     * Subclasses should implement this to enable/disable their connections.
     * @param enabled
     */
    virtual void enabledStateChanged(bool enabled) = 0;


    /**
     * Return connection status text for the current connection state.
     * Implementations of this should check their connections and/or the 
     * state of the enabled/disabled switch to determine an appropriate short
     * text value to print on the connection button.
     */
    virtual String updateButtonText() = 0;

    /**
     * Arranges child components to fit within bounds.
     */
    void resized() override;

    /**
     * If the connection button is clicked this will run openConnectionPage(). 
     * If the switch is clicked, this will call  enabledStateChanged(), passing
     * it the switch toggle state.
     * @param b
     */
    void buttonClicked(Button *b) override;
    
    /**
     * Run refresh() when the component regains visibility.
     */
    void visibilityChanged() override;

    /**
     * Connection button displays status text and opens the connection page
     * when possible.
     */
    class ConnectionButton : public Button
    {
    public:
        /**
         * @param name internal component name
         */
        ConnectionButton(const String& name = String());

        virtual ~ConnectionButton() { }

        /**
         * Sets the text that will be printed on the button.
         * @param text
         */
        void setText(const String &text);

    private:
        /**
         * Draws the connection button outline and prints the button text
         * @param g
         * @param isMouseOverButton
         * @param isButtonDown
         */
        void paintButton
        (Graphics &g, bool isMouseOverButton, bool isButtonDown) override;

        /**
         * Calculates button text height based on button size.
         */
        void resized() override;

        int textHeight;
        int borderSize;
        String displayText;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectionButton)
    };

    DrawableImageComponent icon;
    Spinner spinner;
    SwitchComponent toggle;
    ConnectionButton pageButton;
    std::function<void() > openConnectionPage;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectionSettingsComponent)
};