//  OpenVPN 3 Linux client -- Next generation OpenVPN client
//
//  SPDX-License-Identifier: AGPL-3.0-only
//
//  Copyright (C) 2017 - 2023  OpenVPN Inc <sales@openvpn.net>
//  Copyright (C) 2017 - 2023  David Sommerseth <davids@openvpn.net>
//

/**
 * @file   fetch-config2.cpp
 *
 * @brief  Dumps a specific configuration stored in the configuration manager.
 *         This variant uses the Configuration Manager Proxy object to acheive
 *         this task.  Input to the program is just the D-Bus configuration path
 */

#include <iostream>

#include "dbus/core.hpp"
#include "configmgr/proxy-configmgr.hpp"

using namespace openvpn;



int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <config obj path>" << std::endl;
        return 1;
    }

    //        OpenVPN3ConfigurationProxy config(G_BUS_TYPE_SYSTEM, argv[1]);
    DBus dbusobj(G_BUS_TYPE_SYSTEM);
    dbusobj.Connect();

    OpenVPN3ConfigurationProxy config(dbusobj.GetConnection(), argv[1]);

    std::cout << "Configuration: " << std::endl;
    std::cout << "  - Name:       " << config.GetStringProperty("name") << std::endl;
    std::cout << "  - Read only:  " << (config.GetBoolProperty("readonly") ? "Yes" : "No") << std::endl;
    std::cout << "  - Persistent: " << (config.GetBoolProperty("persistent") ? "Yes" : "No") << std::endl;
    std::cout << "  - Usage:      " << (config.GetBoolProperty("single_use") ? "Once" : "Multiple times") << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << config.GetJSONConfig() << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "** DONE" << std::endl;

    return 0;
}
