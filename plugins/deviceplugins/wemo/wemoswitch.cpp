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

#include "wemoswitch.h"

WemoSwitch::WemoSwitch(QObject *parent, UpnpDeviceDescriptor upnpDeviceDescriptor):
    UpnpDevice(parent, upnpDeviceDescriptor)
{
    m_manager = new QNetworkAccessManager(this);

    connect(m_manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
}

WemoSwitch::~WemoSwitch()
{
}

bool WemoSwitch::powerState()
{
    return m_powerState;
}

bool WemoSwitch::reachable()
{
    return m_reachable;
}

void WemoSwitch::replyFinished(QNetworkReply *reply)
{
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        // clean up
        if (reply == m_setPowerReplay) {
            emit setPowerFinished(false,m_actionId);
            m_setPowerReplay->deleteLater();
        }
        if (reply == m_refrashReplay) {
            m_refrashReplay->deleteLater();
        }
        m_reachable = false;
        emit stateChanged();
        return;
    } else {
        m_reachable = true;
    }

    // if this is the answerer to a refresh request
    if (reply == m_refrashReplay) {
        QByteArray data = reply->readAll();
        if (data.contains("<BinaryState>0</BinaryState>")) {
            m_powerState = false;
        }
        if (data.contains("<BinaryState>1</BinaryState>")) {
            m_powerState = true;
        }
        m_refrashReplay->deleteLater();
    }
    // if this is the answerer to a "set power" request
    if (reply == m_setPowerReplay) {
        QByteArray data = reply->readAll();
        if (data.contains("<BinaryState>1</BinaryState>") || data.contains("<BinaryState>0</BinaryState>")) {
            emit setPowerFinished(true,m_actionId);
        } else {
            emit setPowerFinished(false,m_actionId);
        }
        refresh();
        m_setPowerReplay->deleteLater();
    }

    emit stateChanged();
}

void WemoSwitch::refresh()
{
    QByteArray getBinarayStateMessage("<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:GetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>1</BinaryState></u:GetBinaryState></s:Body></s:Envelope>");

    QNetworkRequest request;
    request.setUrl(QUrl("http://" + hostAddress().toString() + ":" + QString::number(port()) + "/upnp/control/basicevent1"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=\"utf-8\""));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("guh"));
    request.setRawHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#GetBinaryState\"");

    m_refrashReplay = m_manager->post(request,getBinarayStateMessage);
}

bool WemoSwitch::setPower(const bool &power, const ActionId &actionId)
{
    m_actionId = actionId;

    // check if the power state changed...
    if (m_powerState == power) {
        return false;
    }

    QByteArray setPowerMessage("<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>" + QByteArray::number((int)power) + "</BinaryState></u:SetBinaryState></s:Body></s:Envelope>");

    QNetworkRequest request;
    request.setUrl(QUrl("http://" + hostAddress().toString() + ":" + QString::number(port()) + "/upnp/control/basicevent1"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=\"utf-8\""));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("guh"));
    request.setRawHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#SetBinaryState\"");

    m_setPowerReplay = m_manager->post(request,setPowerMessage);
    return true;
}
