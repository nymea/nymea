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

#ifndef TOKENINFO_H
#define TOKENINFO_H

#include <QUuid>
#include <QDateTime>
#include <QMetaType>
#include <QVariant>

class TokenInfo
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QString username READ username)
    Q_PROPERTY(QDateTime creationTime READ creationTime)
    Q_PROPERTY(QString deviceName READ deviceName)

public:
    TokenInfo();
    TokenInfo(const QUuid &id, const QString &username, const QDateTime &creationTime, const QString &deviceName);

    QUuid id() const;
    QString username() const;
    QDateTime creationTime() const;
    QString deviceName() const;

private:
    QUuid m_id;
    QString m_username;
    QDateTime m_creationTime;
    QString m_deviceName;
};


class TokenInfoList: public QList<TokenInfo>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};


Q_DECLARE_METATYPE(TokenInfo)
Q_DECLARE_METATYPE(TokenInfoList)
#endif // TOKENINFO_H
