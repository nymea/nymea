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

#ifndef MAXCUBE_H
#define MAXCUBE_H

#include <QObject>
#include <QTcpSocket>
#include <QDateTime>
#include <QHostAddress>

#include "maxdevice.h"
#include "room.h"

class MaxCube : public QTcpSocket
{
    Q_OBJECT
public:
    MaxCube(QObject *parent = 0, QString serialNumber = QString(), QHostAddress hostAdress = QHostAddress(), quint16 port = 0);

    // cube data access functions
    QString serialNumber();
    void setSerialNumber(QString serialNumber);

    QByteArray rfAddress();
    void setRfAddress(QByteArray rfAddress);

    int firmware();
    void setFirmware(int firmware);

    QHostAddress hostAddress();
    void setHostAddress(QHostAddress hostAddress);

    quint16 port();
    void setPort(quint16 port);

    QByteArray httpConnectionId();
    void setHttpConnectionId(QByteArray httpConnectionId);

    int freeMemorySlots();
    void setFreeMemorySlots(int freeMemorySlots);

    QDateTime cubeDateTime();
    void setCubeDateTime(QDateTime cubeDateTime);

    QList<MaxDevice*> deviceList();
    QList<Room*> roomList();

    void connectToCube();
    void disconnectFromCube();
    bool sendData(QByteArray data);

private:
    // cube data
    QString m_serialNumber;
    QByteArray m_rfAddress;
    int m_firmware;
    QHostAddress m_hostAddress;
    quint16 m_port;
    QByteArray m_httpConnectionId;
    int m_freeMemorySlots;
    QDateTime m_cubeDateTime;

    QList<Room*> m_roomList;
    QList<MaxDevice*> m_deviceList;

    void parseHelloMessage(QByteArray data);
    void parseMetadataMessage(QByteArray data);
    void parseConfigMessage(QByteArray data);
    void parseDevicelistMessage(QByteArray data);
    void parseWeeklyProgram(QByteArray data);
    void parseNewDeviceFoundMessage(QByteArray data);

    QDateTime calculateDateTime(QByteArray dateRaw, QByteArray timeRaw);
    QString deviceTypeString(int deviceType);
    QByteArray fillBin(QByteArray data, int dataLength);

signals:
    void cubeDataAvailable(const QByteArray &data);
    void cubeACK();
    void cubeConnectionStatusChanged(const bool &connected);

private slots:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError error);
    void readData();
    void processCubeData(const QByteArray &data);


public slots:
    void enablePairingMode();
    void disablePairingMode();
    void refresh();
    void customRequest(QByteArray data);

};

#endif // MAXCUBE_H
