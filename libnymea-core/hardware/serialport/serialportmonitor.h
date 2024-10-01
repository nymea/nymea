/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SERIALPORTMONITOR_H
#define SERIALPORTMONITOR_H

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QSerialPortInfo>

#ifdef WITH_UDEV
#include <QSocketNotifier>
#include <libudev.h>
#else
#include <QTimer>
#endif

namespace nymeaserver {

class SerialPort : public QSerialPortInfo {
    Q_GADGET
    Q_PROPERTY(QString systemLocation READ systemLocation)
    Q_PROPERTY(QString manufacturer READ manufacturer)
    Q_PROPERTY(QString description READ customDescription) // Note: give the possibility to use a custom description
    Q_PROPERTY(QString serialNumber READ serialNumber)

public:
    enum SerialPortParity {
        SerialPortParityNoParity = 0,
        SerialPortParityEvenParity = 2,
        SerialPortParityOddParity = 3,
        SerialPortParitySpaceParity = 4,
        SerialPortParityMarkParity = 5,
        SerialPortParityUnknownParity = -1
    };
    Q_ENUM(SerialPortParity)


    enum SerialPortDataBits {
        SerialPortDataBitsData5 = 5,
        SerialPortDataBitsData6 = 6,
        SerialPortDataBitsData7 = 7,
        SerialPortDataBitsData8 = 8,
        SerialPortDataBitsUnknownDataBits = -1
    };
    Q_ENUM(SerialPortDataBits)


    enum SerialPortStopBits {
        SerialPortStopBitsOneStop = 1,
        SerialPortStopBitsOneAndHalfStop = 3,
        SerialPortStopBitsTwoStop = 2,
        SerialPortStopBitsUnknownStopBits = -1
    };
    Q_ENUM(SerialPortStopBits)

    SerialPort() : QSerialPortInfo() { };
    explicit SerialPort(const QSerialPortInfo &other) : QSerialPortInfo(other) { };

    QString customDescription() const {
        // Note: this creats the possibility to override the desciption of
        // serial port for the JSON RPC API.
        if (m_customDescription.isEmpty())
            return description();

        return m_customDescription;
    }

    void setCustomDescription(const QString &customDescription) {
        m_customDescription = customDescription;
    }

private:
    QString m_customDescription;

};

class SerialPorts : public QList<SerialPort>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)

public:
    inline SerialPorts() = default;
    inline SerialPorts(const QList<SerialPort> &other) : QList<SerialPort>(other) { };
    inline bool hasSerialPort(const QString &serialPort) {
        for (int i = 0; i < count(); i++) {
            if (at(i).systemLocation() == serialPort) {
                return true;
            }
        }
        return false;
    };

    inline  Q_INVOKABLE QVariant get(int index) const { return QVariant::fromValue(at(index)); };
    inline Q_INVOKABLE void put(const QVariant &variant) { append(variant.value<SerialPort>()); };
};


class SerialPortMonitor : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortMonitor(QObject *parent = nullptr);

    SerialPorts serialPorts() const;
    bool serialPortAvailable(const QString &systemLocation) const;

signals:
    void serialPortAdded(const SerialPort &serialPort);
    void serialPortRemoved(const SerialPort &serialPort);

private:
    QHash<QString, SerialPort> m_serialPorts;

    void addSerialPortInternally(const SerialPort &serialPort);

#ifdef WITH_UDEV
    struct udev *m_udev = nullptr;
    struct udev_monitor *m_monitor = nullptr;
    QSocketNotifier *m_notifier = nullptr;
#else
    QTimer *m_timer = nullptr;
#endif

};

}

Q_DECLARE_METATYPE(nymeaserver::SerialPort)
Q_DECLARE_METATYPE(nymeaserver::SerialPorts)

#endif // SERIALPORTMONITOR_H
