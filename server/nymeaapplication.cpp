/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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

#ifdef __GLIBC__
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cxxabi.h>
#endif // __GLIBC__

Q_DECLARE_LOGGING_CATEGORY(dcApplication)

namespace nymeaserver {

#ifdef __GLIBC__

static bool s_aboutToShutdown = false;
static bool s_multipleShutdownDetected = false;
static int s_shutdownCounter = 0;

static void catchUnixSignals(const std::vector<int>& quitSignals, const std::vector<int>& ignoreSignals = std::vector<int>())
{
    auto handler = [](int sig) ->void {
        switch (sig) {
        case SIGQUIT:
            qCDebug(dcApplication) << "Cought SIGQUIT quit signal...";
            break;
        case SIGINT:
            qCDebug(dcApplication) << "Cought SIGINT quit signal...";
            break;
        case SIGTERM:
            qCDebug(dcApplication) << "Cought SIGTERM quit signal...";
            break;
        case SIGHUP:
            qCDebug(dcApplication) << "Cought SIGHUP quit signal...";
            break;
        default:
            break;
        }

        if (s_aboutToShutdown) {
            switch (s_shutdownCounter) {
            case 0:
                qCWarning(dcApplication()) << "Already shutting down. Be nice and give me some time to clean up, please.";
                break;
            case 1:
                qCCritical(dcApplication()) << "I told you, I'm already shutting down. Be nice and give me some time to clean up, PLEASE.";
                break;
            case 2:
                qCCritical(dcApplication()) << "Still shutting down...";
                break;
            case 3:
                qCCritical(dcApplication()) << "Hmpf...";
                break;
            case 4:
                qCCritical(dcApplication()) << "It's getting boring...";
                break;
            case 5:
                qCCritical(dcApplication()) << "S H U T T I N G  DOWN";
                break;
            case 6:
                qCCritical(dcApplication()) << "S H U T T I N G  DOWN";
                break;
            case 7:
                qCCritical(dcApplication()) << "S H U T T I N G  DOWN";
                break;
            default:
                qCCritical(dcApplication()) << "Fuck this shit. I'm out...";
                NymeaApplication::quit();
                break;
            }
            s_shutdownCounter++;
            return;
        }

        qCInfo(dcApplication) << "=====================================";
        qCInfo(dcApplication) << "Shutting down nymea daemon";
        qCInfo(dcApplication) << "=====================================";

        s_aboutToShutdown = true;
        NymeaCore::instance()->destroy();

        if (s_multipleShutdownDetected)
            qCDebug(dcApplication) << "Ok, ok, I'm done! :)";

        NymeaApplication::quit();
    };

    // all these signals will be ignored.
    for (int sig : ignoreSignals)
        signal(sig, SIG_IGN);

    for (int sig : quitSignals)
        signal(sig, handler);
}

#endif // __GLIBC__


/*! Constructs a NymeaApplication with the given argument count \a argc and argument vector \a argv. */
NymeaApplication::NymeaApplication(int &argc, char **argv) :
    QCoreApplication(argc, argv)
{

#ifdef __GLIBC__
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
#endif // __GLIBC__
}

}
