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

#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <QString>
#include <QVariant>

class Repository
{
    Q_GADGET
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString displayName READ displayName)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
public:
    Repository();
    Repository(const QString &id, const QString &displayName, bool enabled);

    QString id() const;
    QString displayName() const;

    bool enabled() const;
    void setEnabled(bool enabled);

private:
    QString m_id;
    QString m_displayName;
    bool m_enabled = false;
};
Q_DECLARE_METATYPE(Repository)

class Repositories : public QList<Repository>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Repositories();
    Repositories(const QList<Repository> &other);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

#endif // REPOSITORY_H
