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

#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H

#include <QUuid>
#include <QDebug>
#include <QObject>
#include <QDateTime>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusArgument>

namespace nymeaserver {

typedef QMap<QString, QVariantMap> ConnectionSettings;

class NetworkConnection : public QObject
{
    Q_OBJECT
public:
    explicit NetworkConnection(const QDBusObjectPath &objectPath, QObject *parent = 0);

    void deleteConnection();

    QDBusObjectPath objectPath() const;
    ConnectionSettings connectionSettings() const;

    QString id() const;
    QString name() const;
    QString type() const;
    QUuid uuid() const;
    QString interfaceName() const;
    bool autoconnect() const;
    QDateTime timeStamp() const;

private:
    QDBusObjectPath m_objectPath;
    QDBusInterface *m_connectionInterface;

    ConnectionSettings m_connectionSettings;

};

QDebug operator<<(QDebug debug, NetworkConnection *networkConnection);

}

Q_DECLARE_METATYPE(nymeaserver::ConnectionSettings)


#endif // NETWORKCONNECTION_H
