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

#include "logfilter.h"

namespace guhserver {

LogFilter::LogFilter()
{

}

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

void LogFilter::setStartDate(const QDateTime &startDate)
{
    m_startDate = startDate;
}

QDateTime LogFilter::startDate() const
{
    return m_startDate;
}

void LogFilter::setEndDate(const QDateTime &endDate)
{
    m_endDate = endDate;
}

QDateTime LogFilter::endDate() const
{
    return m_endDate;
}

void LogFilter::addLoggingSource(const Logging::LoggingSource &source)
{
    if (!m_sources.contains(source))
        m_sources.append(source);
}

QList<Logging::LoggingSource> LogFilter::loggingSources() const
{
    return m_sources;
}

void LogFilter::addLoggingLevel(const Logging::LoggingLevel &level)
{
    if (!m_levels.contains(level))
        m_levels.append(level);
}

QList<Logging::LoggingLevel> LogFilter::loggingLevels() const
{
    return m_levels;
}

void LogFilter::addLoggingEventType(const Logging::LoggingEventType &eventType)
{
    if (!m_eventTypes.contains(eventType))
        m_eventTypes.append(eventType);
}

QList<Logging::LoggingEventType> LogFilter::loggingEventTypes() const
{
    return m_eventTypes;
}

void LogFilter::addTypeId(const QUuid &typeId)
{
    if (!m_typeIds.contains(typeId))
        m_typeIds.append(typeId);
}

QList<QUuid> LogFilter::typeIds() const
{
    return m_typeIds;
}

void LogFilter::addDeviceId(const DeviceId &deviceId)
{
    if (!m_deviceIds.contains(deviceId))
        m_deviceIds.append(deviceId);
}

QList<DeviceId> LogFilter::deviceIds() const
{
    return m_deviceIds;
}

void LogFilter::addValue(const QString &value)
{
    if (!m_values.contains(value))
        m_values.append(value);
}

QList<QString> LogFilter::values() const
{
    return m_values;
}

bool LogFilter::isEmpty() const
{
    return m_endDate.isNull() &&
            m_startDate.isNull() &&
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
    if (m_startDate.isValid() && !m_endDate.isValid()) {
        // only start date is valid
        query.append(QString("timestamp BETWEEN '%1' AND '%2' ")
                     .arg(m_startDate.toMSecsSinceEpoch())
                     .arg(QDateTime::currentDateTime().toMSecsSinceEpoch()));
    } else if (!m_startDate.isValid() && m_endDate.isValid()) {
        // only end date is valid
        query.append(QString("timestamp NOT BETWEEN '%1' AND '%2' ")
                     .arg(m_endDate.toMSecsSinceEpoch())
                     .arg(QDateTime::currentDateTime().toMSecsSinceEpoch()));
    } else if (m_startDate.isValid() && m_endDate.isValid()) {
        // both dates are valid
        query.append(QString("timestamp BETWEEN '%1' AND '%2' ")
                     .arg(m_startDate.toMSecsSinceEpoch())
                     .arg(m_endDate.toMSecsSinceEpoch()));
    }
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
