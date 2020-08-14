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
    void addThingId(const ThingId &thingId);
    QList<ThingId> thingIds() const;

    // Valid for LoggingSourceStates
    void addValue(const QString &value);
    QVariantList values() const;

    void setLimit(int limit);
    int limit() const;

    void setOffset(int offset);
    int offset() const;

    bool isEmpty() const;

private:
    QList<QPair<QDateTime, QDateTime > > m_timeFilters;
    QList<Logging::LoggingSource> m_sources;
    QList<Logging::LoggingLevel> m_levels;
    QList<Logging::LoggingEventType> m_eventTypes;
    QList<QUuid> m_typeIds;
    QList<ThingId> m_thingIds;
    QVariantList m_values;
    int m_limit = -1;
    int m_offset = 0;

    QString createDateString() const;
    QString createTimeFilterString(QPair<QDateTime, QDateTime> timeFilter) const;
    QString createSourcesString() const;
    QString createLevelsString() const;
    QString createEventTypesString() const;
    QString createTypeIdsString() const;
    QString createThingIdString() const;
    QString createValuesString() const;
};

}

#endif
