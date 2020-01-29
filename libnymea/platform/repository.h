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

class Repositories: public QList<Repository>
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
