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

#ifndef MAXCUBE_H
#define MAXCUBE_H

#include <QObject>
#include <QTcpSocket>
#include <QDateTime>
#include <QHostAddress>

#include "maxdevice.h"
#include "room.h"
#include "wallthermostat.h"
#include "radiatorthermostat.h"
#include "plugin/deviceplugin.h"

class MaxCube : public QTcpSocket
{
    Q_OBJECT
public:
    MaxCube(QObject *parent = 0, QString serialNumber = QString(), QHostAddress hostAdress = QHostAddress(), quint16 port = 0);

    enum WeekDay{
        Saturday = 0,
        Sunday = 1,
        Monday = 2,
        Tuesday = 3,
        Wednesday = 4,
        Thursday = 5,
        Friday = 6
    };

    // cube data access functions
    QString serialNumber() const;
    void setSerialNumber(const QString &serialNumber);

    QByteArray rfAddress() const;
    void setRfAddress(const QByteArray &rfAddress);

    int firmware() const;
    void setFirmware(const int &firmware);

    QHostAddress hostAddress() const;
    void setHostAddress(const QHostAddress &hostAddress);

    quint16 port() const;
    void setPort(const quint16 &port);

    QDateTime cubeDateTime() const;
    void setCubeDateTime(const QDateTime &cubeDateTime);

    bool portalEnabeld() const;

    QList<WallThermostat*> wallThermostatList();
    QList<RadiatorThermostat*> radiatorThermostatList();

    QList<Room*> roomList();

    void connectToCube();
    void disconnectFromCube();
    bool sendData(QByteArray data);

    bool isConnected();
    bool isInitialized();

private:
    // cube data
    QString m_serialNumber;
    QByteArray m_rfAddress;
    int m_firmware;
    QHostAddress m_hostAddress;
    quint16 m_port;
    QDateTime m_cubeDateTime;
    bool m_portalEnabeld;

    QList<Room*> m_roomList;
    QList<WallThermostat*> m_wallThermostatList;
    QList<RadiatorThermostat*> m_radiatorThermostatList;

    bool m_cubeInitialized;

    void decodeHelloMessage(QByteArray data);
    void decodeMetadataMessage(QByteArray data);
    void decodeConfigMessage(QByteArray data);
    void decodeDevicelistMessage(QByteArray data);
    void decodeCommandMessage(QByteArray data);
    void parseWeeklyProgram(QByteArray data);
    void decodeNewDeviceFoundMessage(QByteArray data);

    QDateTime calculateDateTime(QByteArray dateRaw, QByteArray timeRaw);
    QString deviceTypeString(int deviceType);
    QString weekDayString(int weekDay);

    QByteArray fillBin(QByteArray data, int dataLength);
    QList<QByteArray> splitMessage(QByteArray data);
    int deviceTypeFromRFAddress(QByteArray rfAddress);

    ActionId m_actionId;

signals:
    void cubeDataAvailable(const QByteArray &data);
    void cubeACK();
    void cubeConnectionStatusChanged(const bool &connected);

    // when things are parsed
    void cubeConfigReady();
    void wallThermostatFound();
    void radiatorThermostatFound();

    void wallThermostatDataUpdated();
    void radiatorThermostatDataUpdated();

    void commandActionFinished(const bool &succeeded, const ActionId &actionId);

private slots:
    void connectionStateChanged(const QAbstractSocket::SocketState &socketState);
    void error(QAbstractSocket::SocketError error);
    void readData();
    void processCubeData(const QByteArray &data);


public slots:
    void enablePairingMode();
    void disablePairingMode();
    void refresh();
    void customRequest(QByteArray data);

    // for actions
    void setDeviceSetpointTemp(QByteArray rfAddress, int roomId, double temperature, ActionId actionId);
    void setDeviceAutoMode(QByteArray rfAddress, int roomId, ActionId actionId);
    void setDeviceManuelMode(QByteArray rfAddress, int roomId, ActionId actionId);
    void setDeviceEcoMode(QByteArray rfAddress, int roomId, ActionId actionId);
    void displayCurrentTemperature(QByteArray rfAddress, int roomId, bool display, ActionId actionId);

};

#endif // MAXCUBE_H
