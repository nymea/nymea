/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include <QObject>
#include <QUuid>
#include <QQmlParserStatus>

#include "types/event.h"
#include "devices/devicemanager.h"

namespace nymeaserver {

class ScriptParams;

class ScriptEvent: public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString eventTypeId READ eventTypeId WRITE setEventTypeId NOTIFY eventTypeIdChanged)
    Q_PROPERTY(QString eventName READ eventName WRITE setEventName NOTIFY eventNameChanged)
public:
    ScriptEvent(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString deviceId() const;
    void setDeviceId(const QString &deviceId);

    QString eventTypeId() const;
    void setEventTypeId(const QString &eventTypeId);

    QString eventName() const;
    void setEventName(const QString &eventName);

private slots:
    void onEventTriggered(const Event &event);

signals:
    void deviceIdChanged();
    void eventTypeIdChanged();
    void eventNameChanged();

//    void triggered(ScriptParams *params);
    void triggered(const QVariantMap &params);

private:
    DeviceManager *m_deviceManager = nullptr;

    QString m_deviceId;
    QString m_eventTypeId;
    QString m_eventName;
};

}

#endif // EVENTLISTENER_H
