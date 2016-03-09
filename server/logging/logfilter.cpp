/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

/*!
    \class guhserver::LogFilter
    \brief  Represents a filter to access the logging databse.

    \ingroup logs
    \inmodule core

    A \l{LogFilter} can be used to get \l{LogEntry}{LogEntries} from the \l{LogEngine} matching
    a certain pattern.

    \sa LogEngine, LogEntry, LogsResource, LoggingHandler
*/

#include "logfilter.h"
#include "loggingcategories.h"

namespace guhserver {

/*! Constructs a new \l{LogFilter}.*/
LogFilter::LogFilter()
{

}

/*! Returns the database query string for this \l{LogFilter}.*/
QString LogFilter::queryString() const
{
    if (isEmpty()) {
        return QString();
    }

    QString query;
    query.append(createDateString());

    if (!query.isEmpty() && !m_sources.isEmpty()) {
        query.append("AND ");
    }
    query.append(createSourcesString());

    if (!query.isEmpty() && !m_levels.isEmpty()) {
        query.append("AND ");
    }
    query.append(createLevelsString());

    if (!query.isEmpty() && !m_eventTypes.isEmpty()) {
        query.append("AND ");
    }
    query.append(createEventTypesString());

    if (!query.isEmpty() && !m_typeIds.isEmpty()) {
        query.append("AND ");
    }
    query.append(createTypeIdsString());

    if (!query.isEmpty() && !m_deviceIds.isEmpty()) {
        query.append("AND ");
    }
    query.append(createDeviceIdString());

    if (!query.isEmpty() && !m_values.isEmpty()) {
        query.append("AND ");
    }
    query.append(createValuesString());

    return query;
}

/*! Add a new time filter with the given \a startDate and \a endDate. */
void LogFilter::addTimeFilter(const QDateTime &startDate, const QDateTime &endDate)
{
    QPair<QDateTime, QDateTime> timeFilter(startDate, endDate);
    if (!m_timeFilters.contains(timeFilter))
        m_timeFilters.append(timeFilter);
}

/*! Returns the list of time filters from this \l{LogFilter}. */
QList<QPair<QDateTime, QDateTime> > LogFilter::timeFilters() const
{
    return m_timeFilters;
}

/*! Add a new \a source to this \l{LogFilter}. */
void LogFilter::addLoggingSource(const Logging::LoggingSource &source)
{
    if (!m_sources.contains(source))
        m_sources.append(source);
}

/*! Returns the list of logging sources from this \l{LogFilter}. */
QList<Logging::LoggingSource> LogFilter::loggingSources() const
{
    return m_sources;
}

/*! Add a new \a level to this \l{LogFilter}. */
void LogFilter::addLoggingLevel(const Logging::LoggingLevel &level)
{
    if (!m_levels.contains(level))
        m_levels.append(level);
}

/*! Returns the list of logging levels from this \l{LogFilter}. */
QList<Logging::LoggingLevel> LogFilter::loggingLevels() const
{
    return m_levels;
}

/*! Add a new \a eventType to this \l{LogFilter}. */
void LogFilter::addLoggingEventType(const Logging::LoggingEventType &eventType)
{
    if (!m_eventTypes.contains(eventType))
        m_eventTypes.append(eventType);
}

/*! Returns the list of event types from this \l{LogFilter}. */
QList<Logging::LoggingEventType> LogFilter::loggingEventTypes() const
{
    return m_eventTypes;
}

/*! Add a new \a typeId to this \l{LogFilter}. */
void LogFilter::addTypeId(const QUuid &typeId)
{
    if (!m_typeIds.contains(typeId))
        m_typeIds.append(typeId);
}

/*! Returns the list of type id's from this \l{LogFilter}. */
QList<QUuid> LogFilter::typeIds() const
{
    return m_typeIds;
}

/*! Add a new \a deviceId to this \l{LogFilter}. */
void LogFilter::addDeviceId(const DeviceId &deviceId)
{
    if (!m_deviceIds.contains(deviceId))
        m_deviceIds.append(deviceId);
}

/*! Returns the list of device id's from this \l{LogFilter}. */
QList<DeviceId> LogFilter::deviceIds() const
{
    return m_deviceIds;
}

/*! Add a new \a value to this \l{LogFilter}. */
void LogFilter::addValue(const QString &value)
{
    if (!m_values.contains(value))
        m_values.append(value);
}

/*! Returns the list of values from this \l{LogFilter}. */
QList<QString> LogFilter::values() const
{
    return m_values;
}

/*! Returns true if this \l{LogFilter} is empty. */
bool LogFilter::isEmpty() const
{
    return m_timeFilters.isEmpty() &&
            m_sources.isEmpty() &&
            m_levels.isEmpty() &&
            m_eventTypes.isEmpty() &&
            m_typeIds.isEmpty() &&
            m_deviceIds.isEmpty() &&
            m_values.isEmpty();
}

QString LogFilter::createDateString() const
{
    QString query;
    if (!m_timeFilters.isEmpty()) {
        if (m_timeFilters.count() == 1) {
            QPair<QDateTime, QDateTime> timeFilter = m_timeFilters.first();
            query.append(createTimeFilterString(timeFilter));
        } else {
            query.append("( ");
            QPair<QDateTime, QDateTime> timeFilter;
            foreach (timeFilter, m_timeFilters) {
                query.append(createTimeFilterString(timeFilter));
                if (timeFilter != m_timeFilters.last())
                    query.append("OR ");
            }
            query.append(") ");
        }
    }
    return query;
}

QString LogFilter::createTimeFilterString(QPair<QDateTime, QDateTime> timeFilter) const
{
    QString query;
    QDateTime startDate = timeFilter.first;
    QDateTime endDate = timeFilter.second;

    qCDebug(dcLogEngine) << "create timefiler for" << startDate.toString() << endDate.toString();

    query.append("( ");
    if (startDate.isValid() && !endDate.isValid()) {
        // only start date is valid
        query.append(QString("timestamp BETWEEN '%1' AND '%2' ")
                     .arg(startDate.toTime_t())
                     .arg(QDateTime::currentDateTime().toTime_t()));
    } else if (!startDate.isValid() && endDate.isValid()) {
        // only end date is valid
        query.append(QString("timestamp NOT BETWEEN '%1' AND '%2' ")
                     .arg(endDate.toTime_t())
                     .arg(QDateTime::currentDateTime().toTime_t()));
    } else if (startDate.isValid() && endDate.isValid()) {
        // both dates are valid
        query.append(QString("timestamp BETWEEN '%1' AND '%2' ")
                     .arg(startDate.toTime_t())
                     .arg(endDate.toTime_t()));
    }
    query.append(") ");
    return query;
}

QString LogFilter::createSourcesString() const
{
    QString query;
    if (!m_sources.isEmpty()) {
        if (m_sources.count() == 1) {
            query.append(QString("sourceType = '%1' ").arg(m_sources.first()));
        } else {
            query.append("( ");
            foreach (const Logging::LoggingSource &source, m_sources) {
                query.append(QString("sourceType = '%1' ").arg(source));
                if (source != m_sources.last())
                    query.append("OR ");
            }
            query.append(") ");
        }
    }
    return query;
}

QString LogFilter::createLevelsString() const
{
    QString query;
    if (!m_levels.isEmpty()) {
        if (m_levels.count() == 1) {
            query.append(QString("loggingLevel = '%1' ").arg(m_levels.first()));
        } else {
            query.append("( ");
            foreach (const Logging::LoggingLevel &level, m_levels) {
                query.append(QString("loggingLevel = '%1' ").arg(level));
                if (level != m_levels.last())
                    query.append("OR ");
            }
            query.append(") ");
        }
    }
    return query;
}

QString LogFilter::createEventTypesString() const
{
    QString query;
    if (!m_eventTypes.isEmpty()) {
        if (m_eventTypes.count() == 1) {
            query.append(QString("loggingEventType = '%1' ").arg(m_eventTypes.first()));
        } else {
            query.append("( ");
            foreach (const Logging::LoggingEventType &eventType, m_eventTypes) {
                query.append(QString("loggingEventType = '%1' ").arg(eventType));
                if (eventType != m_eventTypes.last())
                    query.append("OR ");
            }
            query.append(") ");
        }
    }
    return query;
}

QString LogFilter::createTypeIdsString() const
{
    QString query;
    if (!m_typeIds.isEmpty()) {
        if (m_typeIds.count() == 1) {
            query.append(QString("typeId = '%1' ").arg(m_typeIds.first().toString()));
        } else {
            query.append("( ");
            foreach (const QUuid &typeId, m_typeIds) {
                query.append(QString("typeId = '%1' ").arg(typeId.toString()));
                if (typeId != m_typeIds.last())
                    query.append("OR ");
            }
            query.append(") ");
        }
    }
    return query;
}

QString LogFilter::createDeviceIdString() const
{
    QString query;
    if (!m_deviceIds.isEmpty()) {
        if (m_deviceIds.count() == 1) {
            query.append(QString("deviceId = '%1' ").arg(m_deviceIds.first().toString()));
        } else {
            query.append("( ");
            foreach (const DeviceId &deviceId, m_deviceIds) {
                query.append(QString("deviceId = '%1' ").arg(deviceId.toString()));
                if (deviceId != m_deviceIds.last())
                    query.append("OR ");
            }
            query.append(") ");
        }
    }
    return query;
}

QString LogFilter::createValuesString() const
{
    QString query;
    if (!m_values.isEmpty()) {
        if (m_values.count() == 1) {
            query.append(QString("value = '%1' ").arg(m_values.first()));
        } else {
            query.append("( ");
            foreach (const QString &value, m_values) {
                query.append(QString("value = '%1' ").arg(value));
                if (value != m_values.last())
                    query.append("OR ");
            }
            query.append(") ");
        }
    }
    return query;
}

}
