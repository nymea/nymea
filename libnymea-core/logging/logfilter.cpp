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
    \class nymeaserver::LogFilter
    \brief  Represents a filter to access the logging databse.

    \ingroup logs
    \inmodule core

    A \l{LogFilter} can be used to get \l{LogEntry}{LogEntries} from the \l{LogEngine} matching
    a certain pattern.

    \sa LogEngine, LogEntry, LogsResource, LoggingHandler
*/

#include "logfilter.h"
#include "loggingcategories.h"

namespace nymeaserver {

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

    if (!query.isEmpty() && !loggingSources().isEmpty()) {
        query.append("AND ");
    }
    query.append(createSourcesString());

    if (!query.isEmpty() && !loggingLevels().isEmpty()) {
        query.append("AND ");
    }
    query.append(createLevelsString());

    if (!query.isEmpty() && !loggingEventTypes().isEmpty()) {
        query.append("AND ");
    }
    query.append(createEventTypesString());

    if (!query.isEmpty() && !typeIds().isEmpty()) {
        query.append("AND ");
    }
    query.append(createTypeIdsString());

    if (!query.isEmpty() && !thingIds().isEmpty()) {
        query.append("AND ");
    }
    query.append(createThingIdString());

    if (!query.isEmpty() && !values().isEmpty()) {
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

/*! Add a new \a thingId to this \l{LogFilter}. */
void LogFilter::addThingId(const ThingId &thingId)
{
    if (!m_thingIds.contains(thingId))
        m_thingIds.append(thingId);
}

/*! Returns the list of thing id's from this \l{LogFilter}. */
QList<ThingId> LogFilter::thingIds() const
{
    return m_thingIds;
}

/*! Add a new \a value to this \l{LogFilter}. */
void LogFilter::addValue(const QString &value)
{
    if (!m_values.contains(value))
        m_values.append(value);
}

/*! Returns the list of values from this \l{LogFilter}. */
QVariantList LogFilter::values() const
{
    return m_values;
}

/*! Set the maximum count for the result set. Unless a \l{offset} is specified,
 * the newest \a count entries will be returned. \sa{setOffset}
 */
void LogFilter::setLimit(int limit)
{
    m_limit = limit;
}

/*! Returns the maximum count for the result set. \sa{setOffset} */
int LogFilter::limit() const
{
    return m_limit;
}

/*! Set the offset for the result set.
 * The offset starts at the newest entry in the result set.
 * 0 (default) means "all items"
 * Example: If the specified filter returns a total amount of 100 entries:
 * - a offset value of 10 would include the oldest 90 entries
 * - a offset value of 0 would return all 100 entries
 *
 * The offset is particularly useful in combination with the \l{limit} property and
 * can be used for pagination.
 *
 * E.g. A result set of 10000 entries can be fetched in batches of 1000 entries by fetching
 * 1) offset 0, limit 1000: Entries 0 to 9999
 * 2) offset 10000, limit 1000: Entries 10000 - 19999
 * 3) offset 20000, limit 1000: Entries 20000 - 29999
 * ...
 */
void LogFilter::setOffset(int offset)
{
    m_offset = offset;
}

/*! Returns the offset for the result set. \sa{setOffset} */
int LogFilter::offset() const
{
    return m_offset;
}

/*! Returns true if this \l{LogFilter} is empty. */
bool LogFilter::isEmpty() const
{
    return m_timeFilters.isEmpty() &&
            m_sources.isEmpty() &&
            m_levels.isEmpty() &&
            m_eventTypes.isEmpty() &&
            m_typeIds.isEmpty() &&
            m_thingIds.isEmpty() &&
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
                     .arg(startDate.toMSecsSinceEpoch())
                     .arg(QDateTime::currentDateTime().toMSecsSinceEpoch()));
    } else if (!startDate.isValid() && endDate.isValid()) {
        // only end date is valid
        query.append(QString("timestamp NOT BETWEEN '%1' AND '%2' ")
                     .arg(endDate.toMSecsSinceEpoch())
                     .arg(QDateTime::currentDateTime().toMSecsSinceEpoch()));
    } else if (startDate.isValid() && endDate.isValid()) {
        // both dates are valid
        query.append(QString("timestamp BETWEEN '%1' AND '%2' ")
                     .arg(startDate.toMSecsSinceEpoch())
                     .arg(endDate.toMSecsSinceEpoch()));
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

QString LogFilter::createThingIdString() const
{
    QString query;
    if (!m_thingIds.isEmpty()) {
        if (m_thingIds.count() == 1) {
            query.append(QString("thingId = '%1' ").arg(m_thingIds.first().toString()));
        } else {
            query.append("( ");
            foreach (const ThingId &thingId, m_thingIds) {
                query.append(QString("thingId = '%1' ").arg(thingId.toString()));
                if (thingId != m_thingIds.last())
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
            query.append("value = ? ");
        } else {
            query.append("( ");
            foreach (const QVariant &value, m_values) {
                query.append("value = ? ");
                if (value != m_values.last())
                    query.append("OR ");
            }
            query.append(") ");
        }
    }
    return query;
}

}
