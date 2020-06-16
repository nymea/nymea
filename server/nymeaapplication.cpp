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
    and is responsable to catch system signals like SIGQUIT, SIGINT, SIGTERM, SIGHUP, SIGSEGV. This class
    will provide a backtrace on a segmentation fault (SIGSEGV).


    \sa NymeaService
*/

#include "nymeaapplication.h"
#include "loggingcategories.h"
#include "nymeacore.h"

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cxxabi.h>

Q_DECLARE_LOGGING_CATEGORY(dcApplication)

namespace nymeaserver {

static bool s_aboutToShutdown = false;
static bool s_multipleShutdownDetected = false;
static int s_shutdownCounter = 0;

static void printBacktrace()
{
    void* trace[20];

    int traceLength = backtrace(trace, sizeof(trace) / sizeof(void*));
    if (traceLength == 0)
        return;

    char** symbolList = backtrace_symbols(trace, traceLength);
    size_t funktionNameSize = 256;
    char* functionName = (char*)malloc(funktionNameSize);
    for (int i = 1; i < traceLength; i++) {
        QString address = QString().sprintf("%p", trace[i]);
        QString fileName = QString(symbolList[i]);
        int fileIndex = fileName.indexOf("(");
        QString lineCommand = QString("addr2line %1 -e %2").arg(address).arg(fileName.left(fileIndex));

        // Read stdout from addr2line
        char buffer[1024];
        FILE *lineFile = popen(lineCommand.toLatin1().data(), "r");
        QString line(fgets(buffer, sizeof(buffer), lineFile));
        pclose(lineFile);

        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;
        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbolList[i]; *p; ++p) {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }

        QString functionString;
        if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
            int status;
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';
            char* ret = abi::__cxa_demangle(begin_name, functionName, &funktionNameSize, &status);
            if (status == 0) {
                functionName = ret;
                functionString = QString("    %1").arg(QString().sprintf("%s+%s", functionName, begin_offset));
            } else {
                functionString = QString("    %1").arg(QString().sprintf("%s()+%s", begin_name, begin_offset));
            }
        }

#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
        // qCCritical was introduced int Qt 5.2.0
        qCWarning(dcApplication) << QString("[%1] %2").arg(i).arg(symbolList[i]);
        if (!functionString.isEmpty())
            qCWarning(dcApplication) << functionString;

        qCWarning(dcApplication) << QString("    %1").arg(line.remove("\n"));
#else
        qCCritical(dcApplication) << QString("[%1] %2").arg(i).arg(symbolList[i]);
        if (!functionString.isEmpty())
            qCCritical(dcApplication) << functionString;

        qCCritical(dcApplication) << QString("    %1").arg(line.remove("\n"));
#endif // QT_VERSION

    }

    free(functionName);
    free(symbolList);
}

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
        case SIGSEGV: {
            qCWarning(dcApplication) << "Cought SIGSEGV signal. Segmentation fault!";
            printBacktrace();
            exit(1);
        }
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


/*! Constructs a NymeaApplication with the given argument count \a argc and argument vector \a argv. */
NymeaApplication::NymeaApplication(int &argc, char **argv) :
    QCoreApplication(argc, argv)
{
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP, SIGSEGV});
}

}
