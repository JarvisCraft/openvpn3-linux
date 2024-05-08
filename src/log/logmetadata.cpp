//  OpenVPN 3 Linux client -- Next generation OpenVPN client
//
//  SPDX-License-Identifier: AGPL-3.0-only
//
//  Copyright (C)  OpenVPN Inc <sales@openvpn.net>
//  Copyright (C)  David Sommerseth <davids@openvpn.net>
//

/**
 * @file   logmetadata.cpp
 *
 * @brief  Implementation of the LogMetaData and LogMetaDataValue class methods
 */

#include <algorithm>
#include <fstream>
#include <exception>
#include <string>
#include <vector>

#include "logmetadata.hpp"
#include "logtag.hpp"


//
//  LogMetaDataValue  -  implementation
//

LogMetaDataValue::LogMetaDataValue(const std::string &l,
                                   const std::string &v,
                                   bool s)
    : type(Type::LOGMETA_STRING), label(l),
      str_value(v), logtag(nullptr), skip(s)
{
}


LogMetaDataValue::LogMetaDataValue(const std::string &l,
                                   const uint32_t v,
                                   bool s)
    : type(Type::LOGMETA_STRING), label(l),
      str_value(std::to_string(v)), logtag(nullptr), skip(s)
{
}


LogMetaDataValue::LogMetaDataValue(const std::string &l,
                                   const int32_t v,
                                   bool s)
    : type(Type::LOGMETA_STRING), label(l),
      str_value(std::to_string(v)), logtag(nullptr), skip(s)
{
}


LogMetaDataValue::LogMetaDataValue(const std::string &l,
                                   LogTag::Ptr v,
                                   bool s)
    : type(Type::LOGMETA_LOGTAG), label(l),
      str_value(""), logtag(v), skip(s)
{
}


const std::string LogMetaDataValue::GetValue(const bool logtag_encaps) const
{
    switch (type)
    {
    case Type::LOGMETA_STRING:
        return str_value;

    case Type::LOGMETA_LOGTAG:
        return (logtag ? logtag->str(logtag_encaps) : "[INVALID-LOGTAG]");
    }
    return "";
}



//
//  LogMetaData  -  implementation
//

std::string LogMetaData::GetMetaValue(const std::string l,
                                      const bool encaps_logtag,
                                      const std::string postfix) const
{
    auto it = std::find_if(metadata.begin(),
                           metadata.end(),
                           [l](LogMetaDataValue::Ptr e)
                           {
                               return l == e->label;
                           });
    if (metadata.end() == it)
    {
        return "";
    }
    return (*it)->GetValue(encaps_logtag) + postfix;
}


LogMetaData::Records LogMetaData::GetMetaDataRecords(const bool upcase_label,
                                                     const bool logtag_encaps) const
{
    Records ret;
    for (const auto &mdc : metadata)
    {
        std::string label = mdc->label;
        if (upcase_label)
        {
            std::transform(label.begin(),
                           label.end(),
                           label.begin(),
                           [](unsigned char c)
                           {
                               return std::toupper(c);
                           });
        }
        if (LogMetaDataValue::Type::LOGMETA_LOGTAG == mdc->type)
        {
            ret.push_back(label + "=" + mdc->GetValue(logtag_encaps));
        }
        else
        {
            ret.push_back(label + "=" + mdc->GetValue());
        }
    }
    return ret;
}


size_t LogMetaData::size() const
{
    return metadata.size();
}


bool LogMetaData::empty() const
{
    return metadata.empty();
}


void LogMetaData::clear()
{
    metadata.clear();
}


LogMetaData::LogMetaData(const LogMetaData &src)
{
    for (const auto &e : src.metadata)
    {
        metadata.push_back(e);
    }
}
