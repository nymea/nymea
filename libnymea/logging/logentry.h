// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <QVariant>
#include <QDateTime>
#include <QMetaType>

class LogEntry
{
    Q_GADGET
    Q_PROPERTY(QDateTime timestamp READ timestamp)
    Q_PROPERTY(QString source READ source )
    Q_PROPERTY(QVariantMap values READ values)

public:
    LogEntry();
    LogEntry(const QDateTime &timestamp, const QString &source, const QVariantMap &values);

    QDateTime timestamp() const;
    QString source() const;
    QVariantMap values() const;

private:
    QDateTime m_timestamp;
    QString m_source;
    QVariantMap m_values;
};
Q_DECLARE_METATYPE(LogEntry)


class LogEntries: public QList<LogEntry>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    LogEntries();
    LogEntries(const QList<LogEntry> &other);
    LogEntries(std::initializer_list<LogEntry> args):QList(args) {}
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(LogEntries)

#endif // LOGENTRY_H
