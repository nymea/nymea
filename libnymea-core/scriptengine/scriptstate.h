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

#ifndef SCRIPTSTATE_H
#define SCRIPTSTATE_H

#include <QObject>
#include <QQmlParserStatus>
#include <QPointer>

#include "devices/devicemanager.h"
#include "devices/deviceactioninfo.h"

namespace nymeaserver {

class ScriptState : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString stateTypeId READ stateTypeId WRITE setStateTypeId NOTIFY stateTypeChanged)
    Q_PROPERTY(QString stateName READ stateName WRITE setStateName NOTIFY stateTypeChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QVariant minimumValue READ minimumValue NOTIFY stateTypeChanged)
    Q_PROPERTY(QVariant maximumValue READ maximumValue NOTIFY stateTypeChanged)

public:
    explicit ScriptState(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString deviceId() const;
    void setDeviceId(const QString &deviceId);

    QString stateTypeId() const;
    void setStateTypeId(const QString &stateTypeId);

    QString stateName() const;
    void setStateName(const QString &stateName);

    QVariant value() const;
    void setValue(const QVariant &value);

    QVariant minimumValue() const;
    QVariant maximumValue() const;

public slots:
    void store();
    void restore();

signals:
    void deviceIdChanged();
    void stateTypeChanged();
    void valueChanged();

private slots:
    void onDeviceStateChanged(Device *device, const StateTypeId &stateTypeId);

private:
    DeviceManager *m_deviceManager = nullptr;

    QString m_deviceId;
    QString m_stateTypeId;
    QString m_stateName;

    DeviceActionInfo *m_pendingActionInfo = nullptr;
    QVariant m_valueCache;

    QVariant m_valueStore;
};

}

#endif // SCRIPTSTATE_H
