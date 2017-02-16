/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
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
        QtAvahiServiceStateUncomitted,
        QtAvahiServiceStateRegistering,
        QtAvahiServiceStateEstablished,
        QtAvahiServiceStateCollision,
        QtAvahiServiceStateFailure
    };

    explicit QtAvahiService(QObject *parent = 0);
    ~QtAvahiService();

    quint16 port() const;
    QString name() const;
    QString serviceType() const;

    bool registerService(const QString &name, const quint16 &port, const QString &serviceType = "_http._tcp", const QHash<QString, QString> &txt = QHash<QString, QString>());
    void resetService();

    bool isValid() const;
    QString errorString() const;

signals:
    void serviceStateChanged(const QtAvahiServiceState &state);

protected:
    QtAvahiServicePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QtAvahiService)

};

#endif // QTAVAHISERVICE_H
