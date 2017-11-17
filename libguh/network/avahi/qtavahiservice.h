/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
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

#include "libguh.h"

class QtAvahiServicePrivate;

class LIBGUH_EXPORT QtAvahiService : public QObject
{
    Q_OBJECT
    Q_ENUMS(QtAvahiServiceState)

public:
    enum QtAvahiServiceState {
        QtAvahiServiceStateUncomitted = 0,
        QtAvahiServiceStateRegistering = 1,
        QtAvahiServiceStateEstablished = 2,
        QtAvahiServiceStateCollision = 3,
        QtAvahiServiceStateFailure = 4
    };

    explicit QtAvahiService(QObject *parent = nullptr);
    ~QtAvahiService();

    quint16 port() const;
    QString name() const;
    QString serviceType() const;
    QHash<QString, QString> txtRecords() const;
    QtAvahiServiceState state() const;

    bool registerService(const QString &name, const quint16 &port, const QString &serviceType = "_http._tcp", const QHash<QString, QString> &txtRecords = QHash<QString, QString>());
    void resetService();

    bool updateTxtRecord(const QHash<QString, QString> &txtRecords);

    bool isValid() const;
    QString errorString() const;

signals:
    void serviceStateChanged(const QtAvahiServiceState &state);

protected:
    QtAvahiServicePrivate *d_ptr;

private slots:
    bool handlCollision();
    void onStateChanged(const QtAvahiServiceState &state);

private:
    QtAvahiServiceState m_state;
    Q_DECLARE_PRIVATE(QtAvahiService)

};

QDebug operator <<(QDebug dbg, QtAvahiService *service);


#endif // QTAVAHISERVICE_H
