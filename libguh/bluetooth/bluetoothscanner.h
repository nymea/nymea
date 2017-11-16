/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
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

#ifndef BLUETOOTHSCANNER_H
#define BLUETOOTHSCANNER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QtBluetooth>
#include <QBluetoothHostInfo>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>

#include "libguh.h"
#include "typeutils.h"

class LIBGUH_EXPORT BluetoothScanner : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothScanner(const QBluetoothAddress &adapterAddress, QObject *parent = 0);
    bool isRunning();
    bool discover(const PluginId &pluginId);

private:
    QBluetoothAddress m_adapterAddress;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QList<QBluetoothDeviceInfo> m_deviceInfos;
    QTimer *m_timer;
    bool m_available;
    PluginId m_pluginId;

signals:
    void bluetoothDiscoveryFinished(const PluginId &pluginId, const QList<QBluetoothDeviceInfo> &deviceInfos);

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void onError(QBluetoothDeviceDiscoveryAgent::Error error);
    void discoveryTimeout();
};

#endif // BLUETOOTHSCANNER_H
