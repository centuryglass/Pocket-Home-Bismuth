#pragma once
/**
 * @file  DesktopEntry_FormatError.h
 *
 * @brief  Helps prevent invalid data from being added to desktop entry files.
 */

#include <exception>
#include "JuceHeader.h"

namespace DesktopEntry { struct FormatError; }

/**
 * @file  DesktopEntry_FormatError.h
 *
 * @brief  Signals that an attempt was made to add invalid data to an EntryFile
 *         object.
 */
struct DesktopEntry::FormatError : public std::exception
{
public:
    /**
     * @brief  Creates a new exception for an invalid data value.
     *
     * @param badValue  The value that failed to comply with desktop entry
     *                  standards.
     */
    FormatError(const juce::String badValue) :
    badValue(badValue),
    errorMessage(juce::String("Invalid desktop entry value:\"") + badValue
            + "\"")
    { }

    /**
     * @brief  Gets a string describing the error.
     *
     * @return  A non-specific format error message, along with the invalid
     *          value that triggered the exception.
     */
    virtual const char* what() const noexcept override
    {
        return errorMessage.toRawUTF8();
    }

    /**
     * @brief  Gets the value that triggered the exception.
     *
     * @return  The value that failed to comply with desktop entry standards.
     */
    const juce::String& getBadValue() const
    {
        return badValue;
    }

private:
    const juce::String badValue;
    const juce::String errorMessage;
};
