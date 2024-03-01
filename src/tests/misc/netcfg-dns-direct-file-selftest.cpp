//  OpenVPN 3 Linux client -- Next generation OpenVPN client
//
//  SPDX-License-Identifier: AGPL-3.0-only
//
//  Copyright (C) 2018-  OpenVPN Inc <sales@openvpn.net>
//  Copyright (C) 2018-  David Sommerseth <davids@openvpn.net>
//

/**
 * @file   netcfg-dns-direct-file-selftest.cpp
 *
 * @brief  Simple self-test of the NetCfg::DNS::ResolvConfFile() class
 */

#include "build-config.h"

#include <iostream>
#include <cstdio>
#include <sys/stat.h>

#define ENABLE_DEBUG
#include "netcfg/dns/settings-manager.hpp"
#include "netcfg/dns/resolvconf-file.hpp"

using namespace NetCfg::DNS;


bool file_exists(const std::string &fname) noexcept
{
    struct stat buffer;
    return (stat(fname.c_str(), &buffer) == 0);
}


int main()
{
    auto sysresolvconf = ResolvConfFile::Create("/etc/resolv.conf");

    sysresolvconf->Debug_Fetch();
    std::cout << "DUMP OF sysresolvconf" << std::endl
              << sysresolvconf->Dump();
    std::cout << "Writing copy to test-system.conf" << std::endl
              << std::endl;
    sysresolvconf->SetFilename("test-system.conf");
    sysresolvconf->Debug_Write();

    ResolverSettings::Ptr settings = ResolverSettings::Create(1);

    settings->AddSearchDomain("example.org");
    settings->AddSearchDomain("example.com");
    settings->AddNameServer("1.1.1.1");
    settings->AddNameServer("8.8.8.8");
    std::cout << "(Before Enable() call) settings: " << settings << std::endl;
    settings->Enable();
    std::cout << "(After Enable() call)  settings: " << settings << std::endl;
    sysresolvconf->Apply(settings);
    std::cout << "DUMP OF modified sysresolvconf:" << std::endl
              << sysresolvconf->Dump() << std::endl;
    sysresolvconf->SetFilename("modified-system.conf");
    sysresolvconf->Debug_Write();

    auto fresh1 = ResolvConfFile::Create("fresh1.conf");
    ResolverSettings::Ptr settings2 = ResolverSettings::Create(2);

    settings2->ClearSearchDomains();
    settings2->ClearNameServers();
    settings2->AddSearchDomain("example.net");
    settings2->AddNameServer("1.0.0.1");
    settings2->AddNameServer("8.8.4.4");
    settings2->Enable();
    std::cout << "settings2: " << settings2 << std::endl;
    fresh1->Apply(settings2);
    fresh1->Debug_Write();

    std::cout << "DUMP OF fresh1/fresh2" << std::endl
              << fresh1->Dump();
    std::cout << "Writing fresh to fresh2.conf" << std::endl
              << std::endl;
    fresh1->SetFilename("fresh2.conf");
    fresh1->Debug_Write();


    std::cout << std::endl
              << std::endl
              << "== Backup tests == " << std::endl;

    // Create a new file and populate it
    auto backuptest_new = ResolvConfFile::Create("backuptest-start.conf");
    backuptest_new->Apply(settings);
    auto backup_new_servers = backuptest_new->Debug_Get_dns_servers();
    auto backup_new_search = backuptest_new->Debug_Get_search_domains();

    std::cout << "DUMP OF backuptest_new" << std::endl
              << backuptest_new->Dump();
    for (const auto &e : backup_new_servers)
    {
        std::cout << " ~ " << e << std::endl;
    }
    for (const auto &e : backup_new_search)
    {
        std::cout << " ~ " << e << std::endl;
    }

    backuptest_new->Debug_Write();
    backuptest_new->SetFilename("backuptest-new.conf");
    backuptest_new->Apply(settings);
    backuptest_new->Debug_Write();
    backuptest_new.reset();

    std::cout << "backuptest-start.conf file created? "
              << (file_exists("backuptest-start.conf") ? "yes" : "NO!!!")
              << std::endl;


    // Reopen the backup test file and modify it
    auto backuptest_start = ResolvConfFile::Create("backuptest-start.conf",
                                                   "backuptest-backup.conf");
    backuptest_start->Debug_Fetch();
    std::cout << "DUMP OF backuptest_start [1] " << std::endl
              << backuptest_start->Dump();
    backuptest_start->Apply(settings2);
    std::cout << "DUMP OF backuptest_start [2] " << std::endl
              << backuptest_start->Dump();
    backuptest_start->Commit(nullptr);

    std::cout << "Backup file created? "
              << (file_exists("backuptest-backup.conf") ? "yes" : "NO!!!")
              << std::endl;

    auto backup_chk = ResolvConfFile::Create("backuptest-backup.conf");
    backup_chk->Debug_Fetch();

    auto backup_chk_servers = backup_chk->Debug_Get_dns_servers();
    std::cout << "Backup content of DNS servers matches? (original backuptest-start.conf == backuptest-backup.conf) "
              << (backup_new_servers == backup_chk_servers ? "yes" : "NO!!!")
              << std::endl;

    for (const auto &e : backup_chk_servers)
    {
        std::cout << " = " << e << std::endl;
    }

    auto backup_chk_search = backup_chk->Debug_Get_search_domains();
    std::cout << "Backup content of domain search matches? (original backuptest-start.conf == backuptest-backup.conf) "
              << (backup_new_servers == backup_chk_servers ? "yes" : "NO!!!")
              << std::endl;

    for (const auto &e : backup_chk_search)
    {
        std::cout << " = " << e << std::endl;
    }

    backup_chk.reset();
    backuptest_start.reset();

    std::cout << "Backup file removed? "
              << (file_exists("backuptest-backup.conf") ? "NO!!!" : "yes")
              << std::endl;

    return 0;
}
