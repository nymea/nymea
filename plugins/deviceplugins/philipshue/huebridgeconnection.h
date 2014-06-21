/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#ifndef HUEBRIDGECONNECTION_H
#define HUEBRIDGECONNECTION_H

#include <QObject>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QPointer>

class Caller
{
public:
    QPointer<QObject> obj;
    QString method;
    int id;
};

class HueBridgeConnection : public QObject
{
    Q_OBJECT
public:
    explicit HueBridgeConnection(QObject *parent = 0);

    int createUser(const QHostAddress &address, const QString &username);

    int get(const QHostAddress &address, const QString &username, const QString &path, QObject *caller, const QString &methodName);
    int put(const QHostAddress &address, const QString &username, const QString &path, const QVariantMap &data, QObject *caller, const QString &methodName);

private slots:
    void slotCreateUserFinished();
    void slotGetFinished();

signals:
    void createUserFinished(int id, const QVariantMap &map);
    void getFinished(int id, const QVariantMap &map);

private:
    QNetworkAccessManager *m_nam;
    int m_requestCounter;
    QHash<QNetworkReply*, int> m_createUserMap;
    QHash<QNetworkReply*, Caller> m_requestMap;
};

#endif // HUEBRIDGECONNECTION_H
