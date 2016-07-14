/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef LOGENTRY_H
#define LOGENTRY_H

#include "logging.h"
#include "typeutils.h"

#include <QObject>
#include <QDateTime>

namespace guhserver {

class LogEntry
{
    Q_GADGET

public:
    LogEntry(QDateTime timestamp, Logging::LoggingLevel level, Logging::LoggingSource source, int errorCode = 0);
    LogEntry(Logging::LoggingLevel level, Logging::LoggingSource source, int errorCode = 0);
    LogEntry(Logging::LoggingSource source);

    // Valid for all LoggingSources
    QDateTime timestamp() const;
    Logging::LoggingLevel level() const;
    Logging::LoggingSource source() const;

    Logging::LoggingEventType eventType() const;
    void setEventType(const Logging::LoggingEventType &eventType);

    // Valid for LoggingSourceStates, LoggingSourceEvents, LoggingSourceActions, LoggingSourceRules
    QUuid typeId() const;
    void setTypeId(const QUuid &typeId);

    // Valid for LoggingSourceStates, LoggingSourceEvents, LoggingSourceActions
    DeviceId deviceId() const;
    void setDeviceId(const DeviceId &deviceId);

    // Valid for LoggingSourceStates
    QString value() const;
    void setValue(const QString &value);

    // Valid for LoggingEventTypeActiveChanged
    bool active() const;
    void setActive(bool active);

    // Valid for LoggingLevelAlert
    int errorCode() const;

private:
    QDateTime m_timestamp;
    Logging::LoggingLevel m_level;
    Logging::LoggingSource m_source;

    // RuleSource specific properties.
    // FIXME: If it turns out we need many more of those, we should subclass LogEntry with specific ones.
    QUuid m_typeId;
    DeviceId m_deviceId;
    QString m_value;
    Logging::LoggingEventType m_eventType;
    bool m_active;
    int m_errorCode;
};

QDebug operator<<(QDebug dbg, const LogEntry &entry);

}

#endif
