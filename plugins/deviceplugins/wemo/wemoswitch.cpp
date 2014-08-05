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

WemoSwitch::WemoSwitch(QObject *parent) :
    QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);

    connect(m_manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
}

void WemoSwitch::setLocation(const QUrl &location)
{
    m_location = location;
}

QUrl WemoSwitch::location() const
{
    return m_location;
}

void WemoSwitch::setHostAddress(const QHostAddress &hostAddress)
{
    m_hostAddress = hostAddress;
}

QHostAddress WemoSwitch::hostAddress() const
{
    return m_hostAddress;
}

void WemoSwitch::setPort(const int &port)
{
    m_port = port;
}

int WemoSwitch::port() const
{
    return m_port;
}

void WemoSwitch::setManufacturer(const QString &manufacturer)
{
    m_manufacturer = manufacturer;
}

QString WemoSwitch::manufacturer() const
{
    return m_manufacturer;
}

void WemoSwitch::setName(const QString &name)
{
    m_name = name;
}

QString WemoSwitch::name() const
{
    return m_name;
}

void WemoSwitch::setDeviceType(const QString &deviceType)
{
    m_deviceType = deviceType;
}

QString WemoSwitch::deviceType() const
{
    return m_deviceType;
}

void WemoSwitch::setModelDescription(const QString &modelDescription)
{
    m_modelDescription = modelDescription;
}

QString WemoSwitch::modelDescription() const
{
    return m_modelDescription;
}

void WemoSwitch::setModelName(const QString &modelName)
{
    m_modelName = modelName;
}

QString WemoSwitch::modelName() const
{
    return m_modelName;
}

void WemoSwitch::setSerialNumber(const QString &serialNumber)
{
    m_serialNumber = serialNumber;
}

QString WemoSwitch::serialNumber() const
{
    return m_serialNumber;
}

void WemoSwitch::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString WemoSwitch::uuid() const
{
    return m_uuid;
}

bool WemoSwitch::powerState()
{
    return m_powerState;
}

bool WemoSwitch::reachabel()
{
    return m_reachabel;
}

void WemoSwitch::replyFinished(QNetworkReply *reply)
{
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200){
        m_reachabel = false;
        emit stateChanged();
        return;
    }else{
        m_reachabel = true;
    }

    // if this is the answerer to a refresh request
    if(reply == m_refrashReplay){
        QByteArray data = reply->readAll();
        if(data.contains("<BinaryState>0</BinaryState>")){
            m_powerState = false;
        }
        if(data.contains("<BinaryState>1</BinaryState>")){
            m_powerState = true;
        }
    }
    // if this is the answerer to a "set power" request
    if(reply == m_setPowerReplay){
        QByteArray data = reply->readAll();
        if(data.contains("<BinaryState>1</BinaryState>") || data.contains("<BinaryState>0</BinaryState>")){
            emit setPowerFinished(true,m_actionId);
        }else{
            emit setPowerFinished(false,m_actionId);
        }
        refresh();
    }

    emit stateChanged();
}

void WemoSwitch::refresh()
{
    QByteArray getBinarayStateMessage("<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:GetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>1</BinaryState></u:GetBinaryState></s:Body></s:Envelope>");

    QNetworkRequest request;
    request.setUrl(QUrl("http://" + m_hostAddress.toString() + ":" + QString::number(m_port) + "/upnp/control/basicevent1"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=\"utf-8\""));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("guh"));
    request.setRawHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#GetBinaryState\"");

    m_refrashReplay = m_manager->post(request,getBinarayStateMessage);
}

void WemoSwitch::setPower(const bool &power, const ActionId &actionId)
{
    m_actionId = actionId;

    if(m_powerState == power){
        emit setPowerFinished(true,actionId);
    }

    QByteArray setPowerMessage("<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\"><BinaryState>" + QByteArray::number((int)power) + "</BinaryState></u:SetBinaryState></s:Body></s:Envelope>");

    QNetworkRequest request;
    request.setUrl(QUrl("http://" + m_hostAddress.toString() + ":" + QString::number(m_port) + "/upnp/control/basicevent1"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=\"utf-8\""));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("guh"));
    request.setRawHeader("SOAPACTION", "\"urn:Belkin:service:basicevent:1#SetBinaryState\"");

    m_setPowerReplay = m_manager->post(request,setPowerMessage);
}
