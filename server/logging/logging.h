/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef LOGGING_H
#define LOGGING_H

#include <QObject>

namespace guhserver {

class Logging
{
    Q_GADGET
    Q_ENUMS(LoggingError)
    Q_ENUMS(LoggingSource)
    Q_FLAGS(LoggingSources)
    Q_ENUMS(LoggingLevel)
    Q_ENUMS(LoggingEventType)

public:
    enum LoggingError {
        LoggingErrorNoError,
        LoggingErrorLogEntryNotFound,
        LoggingErrorInvalidFilterParameter
    };

    enum LoggingSource {
        LoggingSourceSystem,
        LoggingSourceEvents,
        LoggingSourceActions,
        LoggingSourceStates,
        LoggingSourceRules
    };
    Q_DECLARE_FLAGS(LoggingSources, LoggingSource)

    enum LoggingLevel {
        LoggingLevelInfo,
        LoggingLevelAlert
    };

    enum LoggingEventType {
        LoggingEventTypeTrigger,
        LoggingEventTypeActiveChange,
        LoggingEventTypeEnabledChange,
        LoggingEventTypeActionsExecuted,
        LoggingEventTypeExitActionsExecuted
    };

    Logging(QObject *parent = 0);
};

}

#endif // LOGGING_H
