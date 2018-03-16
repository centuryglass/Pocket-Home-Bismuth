#include "TempTimer.h"
#include "Utils.h"
#include "SwitchComponent.h"

SwitchComponent::SwitchComponent()
{
#    if JUCE_DEBUG
    setName("Switch Component");
#    endif
    addAndMakeVisible(handle);
    handle.setInterceptsMouseClicks(false,false);
}

/**
 * Behaves exactly like the same method in ToggleButton, except it also
 * updates the button handle.
 */
void SwitchComponent::setToggleState(bool shouldBeOn,
        NotificationType notification, bool animate)
{
    if (shouldBeOn != getToggleState())
    {
        if (animate)
        {
            ComponentAnimator& animator = Desktop::getInstance().getAnimator();
            if (animator.isAnimating(&handle))
            {
                TempTimer::initTimer(animationDuration,
                        [this, shouldBeOn, notification]()
                        {
                            clicked();
                            ToggleButton::setToggleState
                                    (shouldBeOn, notification);
                        });
            }
            else
            {
                clicked();
                ToggleButton::setToggleState(shouldBeOn, notification);
            }
        }
        else
        {
            ToggleButton::setToggleState(shouldBeOn, notification);
            resized();
        }
    }
}

/**
 * Draws the switch background as a rounded rectangle.
 */
void SwitchComponent::paintButton
(Graphics &g, bool isMouseOverButton, bool isButtonDown)
{
    float radius = float(backgroundShape.getHeight()) / 2.0f;

    g.setColour(findColour(backgroundColourId));
    g.fillRoundedRectangle(backgroundShape.getX(),
            backgroundShape.getY(),
            backgroundShape.getWidth(),
            backgroundShape.getHeight(),
            radius);
}

/**
 * Update the switch background and handle shapes to the new bounds,
 * without changing their aspect ratios.
 */
void SwitchComponent::resized()
{
    backgroundShape = getLocalBounds();
    float ratio = (float) backgroundShape.getWidth()
            / (float) backgroundShape.getHeight();
    if (ratio > widthByHeight) // too wide, reduce width
    {
        backgroundShape.reduce((backgroundShape.getWidth()
                                - (backgroundShape.getHeight()
                                   * widthByHeight)) / 2, 0);
    }
    else if (ratio < widthByHeight)// too tall, reduce height
    {
        backgroundShape.reduce(0,
                (backgroundShape.getHeight() - backgroundShape.getWidth()
                 / widthByHeight) / 2);
    }
    int handleMargin = std::max(backgroundShape.getHeight() * 0.04, 2.0);
    int handleSize = std::max(backgroundShape.getHeight()
            - handleMargin * 2, 2);
    handleBoundsOff.setBounds(
            backgroundShape.getX() + handleMargin,
            backgroundShape.getY() + handleMargin,
            handleSize,
            handleSize);
    handleBoundsOn = handleBoundsOff.withRightX
            (backgroundShape.getRight() - handleMargin);
    Colour handleColour;
    Rectangle<int>& handleBounds = handleBoundsOff;
    if (getToggleState())
    {
        handleColour = findColour(handleColourId);
        handleBounds = handleBoundsOn;
    }
    else
    {
        handleColour = findColour(handleOffColourId);
    }
    handle.setColour(handleColour);
    handle.setBounds(handleBounds);
}

/**
 * Animates the transition between on and off states, moving the handle
 * left or right as appropriate.
 */
void SwitchComponent::clicked()
{
    const Rectangle<int>& bounds = getToggleState() ?
            handleBoundsOn : handleBoundsOff;
    Desktop::getInstance().getAnimator().animateComponent(&handle, bounds, 1.0f,
            animationDuration, false, 0.0, 0.0);
    handle.setColour(findColour(getToggleState() ?
            handleColourId : handleOffColourId));

}

void SwitchComponent::SwitchHandle::paint(Graphics& g)
{
    g.setColour(colour);
    g.fillEllipse(getLocalBounds().toFloat());
}

