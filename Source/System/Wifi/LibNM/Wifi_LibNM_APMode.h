#pragma once
/**
 * @file  Wifi_LibNM_APMode.h
 *
 * @brief  Defines the basic access point types recognized by LibNM.
 */
namespace Wifi { namespace LibNM { enum class APMode; } }

/**
 * @brief  Enumerates all Wifi access point types.
 *
 *  Any combination of APMode and SecurityType values may be used as bitflags
 * in a single byte.
 */
enum class Wifi::LibNM::APMode
{
    unknown        = 0b0000000,
    adhoc          = 0b0000001,
    infrastructure = 0b0000010,
    hotspot        = 0b0000100
};
