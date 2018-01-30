/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef LOGFILTER_H
#define LOGFILTER_H

#include <QPair>
#include <QDateTime>

#include "logging.h"
#include "typeutils.h"

namespace nymeaserver {

class LogFilter
{
public:
    LogFilter();

    QString queryString() const;


    void addTimeFilter(const QDateTime &startDate = QDateTime(), const QDateTime &endDate = QDateTime());
    QList<QPair<QDateTime, QDateTime> > timeFilters() const;

    void addLoggingSource(const Logging::LoggingSource &source) ;
    QList<Logging::LoggingSource> loggingSources() const;

    void addLoggingLevel(const Logging::LoggingLevel &level);
    QList<Logging::LoggingLevel> loggingLevels() const;

    void addLoggingEventType(const Logging::LoggingEventType &eventType);
    QList<Logging::LoggingEventType> loggingEventTypes() const;

    // Valid for LoggingSourceStates, LoggingSourceEvents, LoggingSourceActions, LoggingSourceRules
    void addTypeId(const QUuid &typeId);
    QList<QUuid> typeIds() const;

    // Valid for LoggingSourceStates, LoggingSourceEvents, LoggingSourceActions
    void addDeviceId(const DeviceId &deviceId);
    QList<DeviceId> deviceIds() const;

    // Valid for LoggingSourceStates
    void addValue(const QString &value);
    QList<QString> values() const;

    bool isEmpty() const;

private:
    QList<QPair<QDateTime, QDateTime > > m_timeFilters;
    QList<Logging::LoggingSource> m_sources;
    QList<Logging::LoggingLevel> m_levels;
    QList<Logging::LoggingEventType> m_eventTypes;
    QList<QUuid> m_typeIds;
    QList<DeviceId> m_deviceIds;
    QList<QString> m_values;

    QString createDateString() const;
    QString createTimeFilterString(QPair<QDateTime, QDateTime> timeFilter) const;
    QString createSourcesString() const;
    QString createLevelsString() const;
    QString createEventTypesString() const;
    QString createTypeIdsString() const;
    QString createDeviceIdString() const;
    QString createValuesString() const;
};

}

#endif
