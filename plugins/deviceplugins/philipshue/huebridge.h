/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#ifndef HUEBRIDGE_H
#define HUEBRIDGE_H

#include <QObject>
#include <QHostAddress>

#include "huelight.h"

class HueBridge : public QObject
{
    Q_OBJECT
public:
    explicit HueBridge(QString apiKey, QHostAddress hostAddress, QObject *parent = 0);

    QString name() const;
    void setName(const QString &name);

    QString apiKey() const;
    void setApiKey(const QString &apiKey);

    QHostAddress hostAddress() const;
    void setHostAddress(const QHostAddress &hostAddress);

    QString macAddress() const;
    void setMacAddress(const QString &macAddress);

    QString apiVersion() const;
    void setApiVersion(const QString &apiVersion);

    int zigbeeChannel() const;
    void setZigbeeChannel(const int &zigbeeChannel);

    QList<HueLight *> lights() const;
    void addLight(HueLight *light);

private:
    QString m_apiKey;
    QHostAddress m_hostAddress;
    QString m_name;
    QString m_macAddress;
    QString m_apiVersion;
    int m_zigbeeChannel;

    QList<HueLight *> m_lights;

};

#endif // HUEBRIDGE_H
