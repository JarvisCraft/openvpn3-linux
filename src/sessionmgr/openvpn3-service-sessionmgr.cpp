//  OpenVPN 3 Linux client -- Next generation OpenVPN client
//
//  SPDX-License-Identifier: AGPL-3.0-only
//
//  Copyright (C) 2017 - 2023  OpenVPN Inc <sales@openvpn.net>
//  Copyright (C) 2017 - 2023  David Sommerseth <davids@openvpn.net>
//

#include <gdbuspp/connection.hpp>
#include <gdbuspp/service.hpp>
#include <glib-unix.h>

#include "common/cmdargparser.hpp"
#include "log/ansicolours.hpp"
#include "log/dbus-log.hpp"
#include "log/logwriter.hpp"
#include "log/logwriters/implementations.hpp"
#include "log/proxy-log.hpp"
#include "sessionmgr-service.hpp"

#include <openvpn/common/base64.hpp>

using namespace openvpn;


static int session_manager(ParsedArgs::Ptr args)
{
    std::cout << get_version(args->GetArgv0()) << std::endl;

    // Enable automatic shutdown if the config manager is
    // idling for 1 minute or more.  By idling, it means
    // no VPN sessions are registered.
    unsigned int idle_wait_min = 3;
    if (args->Present("idle-exit"))
    {
        idle_wait_min = std::atoi(args->GetValue("idle-exit", 0).c_str());
    }

    // Open a log destination, if requested
    std::ofstream logfs;
    std::ostream *logfile = nullptr;
    LogWriter::Ptr logwr = nullptr;
    ColourEngine::Ptr colourengine = nullptr;

    if (args->Present("log-file"))
    {
        std::string fname = args->GetValue("log-file", 0);

        if ("stdout:" != fname)
        {
            logfs.open(fname.c_str(), std::ios_base::app);
            logfile = &logfs;
        }
        else
        {
            logfile = &std::cout;
        }

        if (args->Present("colour"))
        {
            colourengine.reset(new ANSIColours());
            logwr.reset(new ColourStreamWriter(*logfile,
                                               colourengine.get()));
        }
        else
        {
            logwr.reset(new StreamLogWriter(*logfile));
        }
    }

    auto dbuscon = DBus::Connection::Create(DBus::BusType::SYSTEM);
    auto sessmgr_srv = DBus::Service::Create<SessionManager::Service>(dbuscon, logwr);
    sessmgr_srv->PrepareIdleDetector(std::chrono::minutes(idle_wait_min));

    unsigned int log_level = 3;
    if (args->Present("log-level"))
    {
        log_level = std::atoi(args->GetValue("log-level", 0).c_str());
    }
    sessmgr_srv->SetLogLevel(log_level);


    sessmgr_srv->Run();

    return 0;
}


int main(int argc, char **argv)
{
    SingleCommand argparser(argv[0], "OpenVPN 3 Session Manager", session_manager);
    argparser.AddVersionOption();
    argparser.AddOption("log-level", "LOG-LEVEL", true, "Log verbosity level (valid values 0-6, default 3)");
    argparser.AddOption("log-file", "FILE", true, "Write log data to FILE.  Use 'stdout:' for console logging.");
    argparser.AddOption("colour", 0, "Make the log lines colourful");
    argparser.AddOption("idle-exit", "MINUTES", true, "How long to wait before exiting if being idle. "
                                                      "0 disables it (Default: 3 minutes)");
    try
    {
        // This program does not require root privileges,
        // so if used - drop those privileges
        drop_root();

        return argparser.RunCommand(simple_basename(argv[0]), argc, argv);
    }
    catch (const LogServiceProxyException &excp)
    {
        std::cout << "** ERROR ** " << excp.what() << std::endl;
        std::cout << "            " << excp.debug_details() << std::endl;
        return 2;
    }
    catch (CommandException &excp)
    {
        std::cout << excp.what() << std::endl;
        return 2;
    }
}
