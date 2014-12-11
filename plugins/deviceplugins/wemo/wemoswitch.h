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
#include "hardware/upnpdiscovery/upnpdevice.h"

class WemoSwitch : public UpnpDevice
{
    Q_OBJECT
public:
    explicit WemoSwitch(QObject *parent = 0, UpnpDeviceDescriptor upnpDeviceDescriptor = UpnpDeviceDescriptor());
    ~WemoSwitch();

    bool powerState();
    bool reachable();

private:
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_refrashReplay;
    QNetworkReply *m_setPowerReplay;

    bool m_powerState;
    bool m_reachable;

    ActionId m_actionId;
signals:
    void stateChanged();
    void setPowerFinished(const bool &succeeded, const ActionId &actionId);

private slots:
    void replyFinished(QNetworkReply *reply);

public slots:
    void refresh();
    bool setPower(const bool &power,  const ActionId &actionId);
};

#endif // WEMOSWITCH_H
