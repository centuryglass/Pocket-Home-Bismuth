/**
 * @file SwitchComponent.h
 * 
 * SwitchComponent is a ToggleButton drawn as a round handle that switches
 * left/off or right/on in front of a rounded rectangle.
 */
#pragma once
#include "JuceHeader.h"

class SwitchComponent : public ToggleButton {
public:

    enum ColourIds {
        backgroundColourId = 0x100f000,
        handleColourId = 0x100f001,
        handleOffColourId = 0x100f002
    };

    SwitchComponent();
    virtual ~SwitchComponent() {}

private:
    
    /**
     * Draws the switch background as a rounded rectangle.
     * @param g
     * @param isMouseOverButton
     * @param isButtonDown
     */
    void paintButton(Graphics &g,
            bool isMouseOverButton, bool isButtonDown) override;
    
    /**
     * Update the switch background and handle shapes to the new bounds,
     * without changing their aspect ratios.
     */
    void resized() override;
    
    /**
     * Animates the transition between on and off states, moving the handle
     * left or right as appropriate.
     */
    void clicked() override;
    
    /**
     * Updates the DrawablePath handle's internal path to fit the current
     * bounds and switch position.  This needs to be called whenever the switch
     * changes position or resizes.
     */
    void updateHandlePath();
    
    //defines the background width:height ratio as 1.5:1
    float widthByHeight = 1.5;
    //draws the switch handle as a simple filled circle. 
    DrawablePath handle;
    //defines the bounds of the switch handle in both button states
    Rectangle<int> handleBoundsOff, handleBoundsOn;
    //defines the switch's rounded rectangle background
    Rectangle<int> backgroundShape;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SwitchComponent)
};
