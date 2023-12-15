//  OpenVPN 3 Linux client -- Next generation OpenVPN client
//
//  SPDX-License-Identifier: AGPL-3.0-only
//
//  Copyright (C)  OpenVPN Inc <sales@openvpn.net>
//  Copyright (C)  David Sommerseth <davids@openvpn.net>
//  Copyright (C)  Arne Schwabe <arne@openvpn.net>
//  Copyright (C)  Lev Stipakov <lev@openvpn.net>
//

#pragma once

#include <cstdint>
#include <sstream>
#include <string>
#include <array>


class LogException : public std::exception
{
  public:
    LogException(const std::string &err) noexcept
        : errorstr(err)
    {
        details = errorstr;
    }

    virtual ~LogException() noexcept = default;

    virtual const char *what() const noexcept
    {
        return details.c_str();
    }

  private:
    std::string errorstr;
    std::string details;
};



/**
 * Log groups is used to classify the source of log events
 */
const uint8_t LogGroupCount = 10;
enum class LogGroup : std::uint8_t
{
    UNDEFINED,    /**< Default - should not be used in code, but is here to detect errors */
    MASTERPROC,   /**< Master process (main openvpn-manager) */
    CONFIGMGR,    /**< Configuration Manager process (child of openvpn-manager)*/
    SESSIONMGR,   /**< Session manager process (child of openvpn-manager) */
    BACKENDSTART, /**< Backend starter process (openvpn3-service-backendstart) */
    LOGGER,       /**< Logger process (child of openvpn-manager) */
    BACKENDPROC,  /**< Session process (openvpn-service-client) */
    CLIENT,       /**< OpenVPN 3 Core tunnel object in the session process */
    NETCFG,       /**< Network Configuration service (openvpn3-service-netcfg)*/
    EXTSERVICE    /**< External services integrating with openvpn3-service-logger */
};

const std::array<const std::string, LogGroupCount> LogGroup_str = {
    {"[[UNDEFINED]]",
     "Master Process",
     "Config Manager",
     "Session Manager",
     "Backend Starter",
     "Logger",
     "Backend Session Process",
     "Client",
     "Network Configuration",
     "External Service"}};


enum class LogCategory : uint8_t
{
    UNDEFINED, /**< Undefined/not set */
    DEBUG,     /**< Debug messages */
    VERB2,     /**< Even more details */
    VERB1,     /**< More details */
    INFO,      /**< Informational messages */
    WARN,      /**< Warnings - important issues which might need attention*/
    ERROR,     /**< Errors - These must be fixed for successful operation */
    CRIT,      /**< Critical - These requires users attention */
    FATAL      /**< Fatal errors - The current operation is going to stop */
};

const std::array<const std::string, 9> LogCategory_str = {
    {
        "[[UNDEFINED]]",   // LogCategory::UNDEFINED
        "DEBUG",           // LogFlags::DEBUG
        "VERB2",           // LogFlags::VERB2
        "VERB1",           // LogFlags::VERB1
        "INFO",            // LogFlags::INFO
        "WARNING",         // LogFlags::WARN
        "-- ERROR --",     // LogFlags::ERROR
        "!! CRITICAL !!",  // LogFlags::CRIT
        "**!! FATAL !!**", // LogFlags::FATAL
    }};



inline const std::string LogPrefix(LogGroup group, LogCategory catg)
{
    if (LogGroup::UNDEFINED == group && LogCategory::UNDEFINED == catg)
    {
        return std::string();
    }

    std::stringstream grp_str;
    if ((uint8_t)group >= LogGroupCount)
    {
        grp_str << "[group:" << std::to_string((uint8_t)group) << "]";
    }
    else
    {
        grp_str << LogGroup_str[(uint8_t)group];
    }

    std::stringstream catg_str;
    if ((uint8_t)catg > 8)
    {
        catg_str << "[category:" << std::to_string((uint8_t)catg) << "]";
    }
    else
    {
        catg_str << LogCategory_str[(uint8_t)catg];
    }

    std::stringstream ret;
    if (LogGroup::UNDEFINED != group)
    {
        ret << grp_str.str();
    }
    if (LogCategory::UNDEFINED != catg)
    {
        if (LogGroup::UNDEFINED != group)
        {
            ret << " ";
        }
        ret << catg_str.str();
    }
    ret << ": ";
    return ret.str();
}
