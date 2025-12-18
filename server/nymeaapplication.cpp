// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::NymeaApplication
    \brief Application class of the nymea server.

    \ingroup core
    \inmodule server

    The \l{NymeaApplication} is a subclass of the \{http://doc.qt.io/qt-5/qcoreapplication.html}{QCoreApplication}
    and is responsable to catch system signals like SIGQUIT, SIGINT, SIGTERM, SIGHUP..


    \sa NymeaService
*/

#include "nymeaapplication.h"
#include "loggingcategories.h"
#include "nymeacore.h"

#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

Q_DECLARE_LOGGING_CATEGORY(dcApplication)

namespace nymeaserver {

static int s_shutdownCounter = 0;

static void catchUnixSignals(const std::vector<int> &quitSignals, const std::vector<int> &ignoreSignals = std::vector<int>())
{
    auto handler = [](int sig) -> void {
        // forecefully exit() if repeated signals come in.
        if (s_shutdownCounter > 0) {
            if (s_shutdownCounter < 4) {
                qCCritical(dcApplication()) << "Shutdown in progress." << (4 - s_shutdownCounter) << "more times to abort.";
                s_shutdownCounter++;
                return;
            }
            exit(EXIT_FAILURE);
            return;
        }

        NymeaCore::ShutdownReason reason = NymeaCore::ShutdownReasonQuit;
        switch (sig) {
        case SIGQUIT:
            qCDebug(dcApplication()) << "Cought SIGQUIT signal...";
            reason = NymeaCore::ShutdownReasonQuit;
            break;
        case SIGINT:
            qCDebug(dcApplication()) << "Cought SIGINT signal...";
            reason = NymeaCore::ShutdownReasonTerm;
            break;
        case SIGTERM:
            qCDebug(dcApplication()) << "Cought SIGTERM signal...";
            reason = NymeaCore::ShutdownReasonTerm;
            break;
        case SIGHUP:
            qCDebug(dcApplication()) << "Cought SIGHUP signal...";
            reason = NymeaCore::ShutdownReasonTerm;
            break;
        case SIGKILL:
            qCDebug(dcApplication()) << "Cought SIGKILL signal...";
            reason = NymeaCore::ShutdownReasonTerm;
            break;
        case SIGSEGV:
            qCDebug(dcApplication()) << "Cought SIGSEGV quit signal...";
            reason = NymeaCore::ShutdownReasonFailure;
            break;
        case SIGFPE:
            qCDebug(dcApplication()) << "Cought SIGFPE quit signal...";
            reason = NymeaCore::ShutdownReasonFailure;
            break;
        default:
            qCDebug(dcApplication()) << "Cought signal" << sig;
            break;
        }

        qCInfo(dcApplication()) << "=====================================";
        qCInfo(dcApplication()) << "Shutting down nymea:core";
        qCInfo(dcApplication()) << "=====================================";

        s_shutdownCounter++;
        NymeaCore::instance()->destroy(reason);

        qCInfo(dcApplication()) << "nymea:core shut down successfully";
        NymeaApplication::processEvents();
        NymeaApplication::quit();
    };

    // all these signals will be ignored.
    for (int sig : ignoreSignals)
        signal(sig, SIG_IGN);

    for (int sig : quitSignals)
        signal(sig, handler);
}

/*! Constructs a NymeaApplication with the given argument count \a argc and argument vector \a argv. */
NymeaApplication::NymeaApplication(int &argc, char **argv)
    : QCoreApplication(argc, argv)
{
    // Catching SIGSEGV messes too much with various tools...
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP, SIGKILL, /*SIGSEGV,*/ SIGFPE});
}

NymeaApplication::~NymeaApplication()
{
    quit();
}

} // namespace nymeaserver
