/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2024, nymea GmbH
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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
