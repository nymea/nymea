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

#ifndef LOGENTRY_H
#define LOGENTRY_H

#include "logging.h"
#include "typeutils.h"

#include <QObject>
#include <QVariant>
#include <QDateTime>

namespace nymeaserver {

class LogEntry
{
    Q_GADGET
    Q_PROPERTY(QDateTime timestamp READ timestamp)
    Q_PROPERTY(Logging::LoggingLevel loggingLevel READ level)
    Q_PROPERTY(Logging::LoggingSource source READ source)
    Q_PROPERTY(QUuid typeId READ typeId USER true)
    Q_PROPERTY(QUuid thingId READ thingId USER true)
    Q_PROPERTY(QVariant value READ value USER true)
    Q_PROPERTY(bool active READ active USER true)
    Q_PROPERTY(Logging::LoggingEventType eventType READ eventType USER true)
    Q_PROPERTY(QString errorCode READ errorCode USER true)

public:
    LogEntry();
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
    ThingId thingId() const;
    void setThingId(const ThingId &thingId);

    // Valid for LoggingSourceStates, LoggingSourceBrowserActions
    QVariant value() const;
    void setValue(const QVariant &value);

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
    ThingId m_thingId;
    QVariant m_value;
    Logging::LoggingEventType m_eventType;
    bool m_active;
    int m_errorCode;
};

class LogEntries: QList<LogEntry>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    LogEntries();
    LogEntries(const QList<LogEntry> &other);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

QDebug operator<<(QDebug dbg, const LogEntry &entry);

}
Q_DECLARE_METATYPE(nymeaserver::LogEntry)
Q_DECLARE_METATYPE(nymeaserver::LogEntries)

#endif
