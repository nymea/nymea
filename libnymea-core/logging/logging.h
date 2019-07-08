/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef LOGGING_H
#define LOGGING_H

#include <QObject>

namespace nymeaserver {

class Logging
{
    Q_GADGET

public:
    enum LoggingError {
        LoggingErrorNoError,
        LoggingErrorLogEntryNotFound,
        LoggingErrorInvalidFilterParameter
    };
    Q_ENUM(LoggingError)

    enum LoggingSource {
        LoggingSourceSystem,
        LoggingSourceEvents,
        LoggingSourceActions,
        LoggingSourceStates,
        LoggingSourceRules
    };
    Q_ENUM(LoggingSource)
    Q_FLAGS(LoggingSources)
    Q_DECLARE_FLAGS(LoggingSources, LoggingSource)

    enum LoggingLevel {
        LoggingLevelInfo,
        LoggingLevelAlert
    };
    Q_ENUM(LoggingLevel)

    enum LoggingEventType {
        LoggingEventTypeTrigger,
        LoggingEventTypeActiveChange,
        LoggingEventTypeEnabledChange,
        LoggingEventTypeActionsExecuted,
        LoggingEventTypeExitActionsExecuted
    };
    Q_ENUM(LoggingEventType)

    Logging(QObject *parent = nullptr);
};

}

#endif // LOGGING_H
