/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef QTAVAHISERVICE_H
#define QTAVAHISERVICE_H

#include <QHash>
#include <QString>
#include <QObject>
#include <QHostAddress>
#include <QTimer>

namespace nymeaserver {

class QtAvahiServicePrivate;

class QtAvahiService : public QObject
{
    Q_OBJECT

public:
    enum QtAvahiServiceState {
        QtAvahiServiceStateUncommitted = 0,
        QtAvahiServiceStateRegistering = 1,
        QtAvahiServiceStateEstablished = 2,
        QtAvahiServiceStateCollision = 3,
        QtAvahiServiceStateFailure = 4
    };
    Q_ENUM(QtAvahiServiceState)

    explicit QtAvahiService(QObject *parent = nullptr);
    ~QtAvahiService();

    QHostAddress hostAddress() const;
    quint16 port() const;
    QString name() const;
    QString serviceType() const;
    QHash<QString, QString> txtRecords() const;
    QtAvahiServiceState state() const;

    bool registerService(const QString &name, const QHostAddress &hostAddress, const quint16 &port, const QString &serviceType = "_http._tcp", const QHash<QString, QString> &txtRecords = QHash<QString, QString>(), bool silent = false);
    void resetService(bool silent = false);

    bool updateTxtRecord(const QHash<QString, QString> &txtRecords);

    bool isValid() const;
    QString errorString() const;

signals:
    void serviceStateChanged(const QtAvahiServiceState &state);

protected:
    QtAvahiServicePrivate *d_ptr;

private slots:
    bool handleCollision();
    void onStateChanged(const QtAvahiServiceState &state);

private:
    QtAvahiServiceState m_state;
    Q_DECLARE_PRIVATE(QtAvahiService)

    QTimer m_reregisterTimer;

    QString m_name;
    QHostAddress m_hostAddress;
    quint16 m_port;
    QString m_serviceType;
    QHash<QString, QString> m_txtRecords;
};

QDebug operator <<(QDebug dbg, QtAvahiService *service);

}

#endif // QTAVAHISERVICE_H
