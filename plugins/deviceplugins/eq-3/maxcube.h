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
#include "livemessage.h"

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

    QByteArray httpConnectionId() const;
    void setHttpConnectionId(const QByteArray &httpConnectionId);

    int freeMemorySlots() const;
    void setFreeMemorySlots(const int &freeMemorySlots);

    QDateTime cubeDateTime() const;
    void setCubeDateTime(const QDateTime &cubeDateTime);

    bool portalEnabeld() const;

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
    bool m_portalEnabeld;

    QList<Room*> m_roomList;
    QList<MaxDevice*> m_deviceList;

    bool m_cubeInitialized;

    void decodeHelloMessage(QByteArray data);
    void decodeMetadataMessage(QByteArray data);
    void decodeConfigMessage(QByteArray data);
    void decodeDevicelistMessage(QByteArray data);
    void parseWeeklyProgram(QByteArray data);
    void decodeNewDeviceFoundMessage(QByteArray data);

    QDateTime calculateDateTime(QByteArray dateRaw, QByteArray timeRaw);
    QString deviceTypeString(int deviceType);
    QString weekDayString(int weekDay);

    QByteArray fillBin(QByteArray data, int dataLength);
    QList<QByteArray> splitMessage(QByteArray data);
    MaxDevice *getDeviceTypeFromRFAddress(QByteArray rfAddress);

signals:
    void cubeDataAvailable(const QByteArray &data);
    void cubeACK();
    void cubeConnectionStatusChanged(const bool &connected);
    void deviceListReady(QList<MaxDevice*> maxDeviceList);
    void liveMessageReady(const LiveMessage &liveMessage);

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
