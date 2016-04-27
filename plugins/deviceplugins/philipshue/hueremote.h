/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef HUEREMOTE_H
#define HUEREMOTE_H

#include <QObject>
#include <QDebug>
#include <QHostAddress>
#include <QNetworkRequest>
#include <QJsonDocument>

#include "typeutils.h"
#include "huedevice.h"

class HueRemote : public HueDevice
{
    Q_OBJECT
public:
    enum ButtonCode {
        OnLongPressed = 1001,
        OnPressed = 1002,
        DimUpLongPressed = 2001,
        DimUpPressed = 2002,
        DimDownLongPressed = 3001,
        DimDownPressed = 3002,
        OffLongPressed = 4001,
        OffPressed = 4002
    };

    explicit HueRemote(QObject *parent = 0);

    int battery() const;
    void setBattery(const int &battery);

    void updateStates(const QVariantMap &statesMap, const QVariantMap &configMap);

private:
    int m_battery;
    QString m_lastUpdate;

signals:
    void stateChanged();
    void buttonPressed(const int &buttonCode);

};

#endif // HUEREMOTE_H
