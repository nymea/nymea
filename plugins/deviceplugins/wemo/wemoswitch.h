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

#ifndef WEMOSWITCH_H
#define WEMOSWITCH_H

#include <QObject>
#include <QHostAddress>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>

#include "plugin/deviceplugin.h"

class WemoSwitch : public QObject
{
    Q_OBJECT
public:
    explicit WemoSwitch(QObject *parent = 0);

    void setLocation(const QUrl &location);
    QUrl location() const;

    void setHostAddress(const QHostAddress &hostAddress);
    QHostAddress hostAddress() const;

    void setPort(const int &port);
    int port() const;

    void setManufacturer(const QString &manufacturer);
    QString manufacturer() const;

    void setName(const QString &name);
    QString name() const;

    void setDeviceType(const QString &deviceType);
    QString deviceType() const;

    void setModelDescription(const QString &modelDescription);
    QString modelDescription() const;

    void setModelName(const QString &modelName);
    QString modelName() const;

    void setSerialNumber(const QString &serialNumber);
    QString serialNumber() const;

    void setUuid(const QString &uuid);
    QString uuid() const;

    bool powerState();
    bool reachabel();

private:
    QUrl m_location;
    QHostAddress m_hostAddress;
    int m_port;
    QString m_name;
    QString m_deviceType;
    QString m_modelName;
    QString m_modelDescription;
    QString m_manufacturer;
    QString m_serialNumber;
    QString m_uuid;

    QNetworkAccessManager *m_manager;
    QNetworkReply *m_refrashReplay;
    QNetworkReply *m_setPowerReplay;

    bool m_powerState;
    bool m_reachabel;

    ActionId m_actionId;
signals:
    void stateChanged();
    void setPowerFinished(const bool &succeeded, const ActionId &actionId);

private slots:
    void replyFinished(QNetworkReply *reply);

public slots:
    void refresh();
    void setPower(const bool &power,  const ActionId &actionId);
};

#endif // WEMOSWITCH_H
