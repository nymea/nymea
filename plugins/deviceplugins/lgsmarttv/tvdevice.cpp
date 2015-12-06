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

#include "tvdevice.h"
#include "extern-plugininfo.h"

TvDevice::TvDevice(const QHostAddress &hostAddress, const int &port, QObject *parent) :
    QObject(parent),
    m_hostAddress(hostAddress),
    m_port(port),
    m_paired(false),
    m_reachable(false),
    m_is3DMode(false),
    m_mute(false),
    m_volumeLevel(-1),
    m_inputSourceIndex(-1),
    m_channelNumber(-1)
{
    m_eventHandler = new TvEventHandler(hostAddress, port, this);

    connect(m_eventHandler, &TvEventHandler::eventOccured, this, &TvDevice::eventOccured);
}

void TvDevice::setKey(const QString &key)
{
    m_key = key;
}

QString TvDevice::key() const
{
    return m_key;
}

void TvDevice::setHostAddress(const QHostAddress &hostAddress)
{
    m_hostAddress = hostAddress;
}

QHostAddress TvDevice::hostAddress() const
{
    return m_hostAddress;
}

void TvDevice::setPort(const int &port)
{
    m_port = port;
}

int TvDevice::port() const
{
    return m_port;
}

void TvDevice::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString TvDevice::uuid() const
{
    return m_uuid;
}

void TvDevice::setPaired(const bool &paired)
{
    if (m_paired != paired) {
        m_paired = paired;
        stateChanged();
    }
}

bool TvDevice::paired() const
{
    return m_paired;
}

void TvDevice::setReachable(const bool &reachable)
{
    if (m_reachable != reachable) {
        m_reachable = reachable;
        emit stateChanged();
    }
}

bool TvDevice::reachable() const
{
    return m_reachable;
}

bool TvDevice::is3DMode() const
{
    return m_is3DMode;
}

int TvDevice::volumeLevel() const
{
    return m_volumeLevel;
}

bool TvDevice::mute() const
{
    return m_mute;
}

QString TvDevice::channelType() const
{
    return m_channelType;
}

QString TvDevice::channelName() const
{
    return m_channelName;
}

int TvDevice::channelNumber() const
{
    return m_channelNumber;
}

QString TvDevice::programName() const
{
    return m_programName;
}

int TvDevice::inputSourceIndex() const
{
    return m_inputSourceIndex;
}

QString TvDevice::inputSourceLabelName() const
{
    return m_inputSourceLabel;
}

QPair<QNetworkRequest, QByteArray> TvDevice::createDisplayKeyRequest(const QHostAddress &host, const int &port)
{
    QString urlString = "http://" + host.toString() + ":" + QString::number(port) + "/udap/api/pairing";
    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0"));

    QByteArray data = "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"pairing\"> <name>showKey</name></api></envelope>";
    return QPair<QNetworkRequest, QByteArray>(request, data);
}

QPair<QNetworkRequest, QByteArray> TvDevice::createPairingRequest(const QHostAddress &host, const int &port, const QString &key)
{
    QString urlString = "http://" + host.toString() + ":" + QString::number(port) + "/udap/api/pairing";
    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0 guh"));

    QByteArray data = "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"pairing\"><name>hello</name><value>" + key.toUtf8() + "</value><port>8080</port></api></envelope>";
    return QPair<QNetworkRequest, QByteArray>(request, data);
}

QPair<QNetworkRequest, QByteArray> TvDevice::createEndPairingRequest(const QUrl &url)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0 guh"));
    request.setRawHeader("Connection", "Close");

    QByteArray data = "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"pairing\"><name>byebye</name><port>8080</port></api></envelope>";
    return QPair<QNetworkRequest, QByteArray>(request, data);
}

QPair<QNetworkRequest, QByteArray> TvDevice::createEndPairingRequest(const QHostAddress &host, const int &port)
{
    QString urlString = "http://" + host.toString() + ":" + QString::number(port) + "/udap/api/pairing";
    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0 guh"));
    request.setRawHeader("Connection", "Close");

    QByteArray data = "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"pairing\"><name>byebye</name><port>8080</port></api></envelope>";
    return QPair<QNetworkRequest, QByteArray>(request, data);
}

QNetworkRequest TvDevice::createVolumeInformationRequest()
{
    QString urlString = "http://" + hostAddress().toString()  + ":" + QString::number(port()) + "/udap/api/data?target=volume_info";
    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0"));
    request.setRawHeader("Connection", "Close");
    return request;
}

QNetworkRequest TvDevice::createChannelInformationRequest()
{
    QString urlString = "http://" + hostAddress().toString()  + ":" + QString::number(port()) + "/udap/api/data?target=cur_channel";
    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0"));
    request.setRawHeader("Connection", "Close");
    return request;
}

QPair<QNetworkRequest, QByteArray> TvDevice::createPressButtonRequest(const TvDevice::RemoteKey &key)
{
    QString urlString = "http://" + hostAddress().toString()  + ":" + QString::number(port()) + "/udap/api/command";
    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0 guh"));

    QByteArray data;
    data.append("<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"command\"><name>HandleKeyInput</name><value>");
    data.append(QString::number(key).toUtf8());
    data.append("</value></api></envelope>");
    return QPair<QNetworkRequest, QByteArray>(request, data);
}

void TvDevice::onVolumeInformationUpdate(const QByteArray &data)
{
    //qCDebug(dcLgSmartTv) << printXmlData(data);
    QXmlStreamReader xml(data);

    while(!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if(xml.name() == "mute") {
            m_mute = QVariant(xml.readElementText()).toBool();
        }
        if(xml.name() == "level") {
            m_volumeLevel = QVariant(xml.readElementText()).toInt();
        }
    }
    emit stateChanged();
}

void TvDevice::onChannelInformationUpdate(const QByteArray &data)
{
    //qCDebug(dcLgSmartTv) << printXmlData(data);
    QXmlStreamReader xml(data);

    while(!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if(xml.name() == "chtype") {
            m_channelType = xml.readElementText();
        }
        if(xml.name() == "major") {
            m_channelNumber = QVariant(xml.readElementText()).toInt();
        }
        if(xml.name() == "chname") {
            m_channelName = xml.readElementText();
        }
        if(xml.name() == "progName") {
            m_programName = xml.readElementText();
        }
        if(xml.name() == "inputSourceIdx") {
            m_inputSourceIndex = QVariant(xml.readElementText()).toInt();
        }
        if(xml.name() == "labelName") {
            m_inputSourceLabel = xml.readElementText();
        }
    }
    emit stateChanged();
}

QString TvDevice::printXmlData(const QByteArray &data)
{
    QString xmlOut;
    QXmlStreamReader reader(data);
    QXmlStreamWriter writer(&xmlOut);
    writer.setAutoFormatting(true);

    while (!reader.atEnd()) {
        reader.readNext();
        if(!reader.isWhitespace()) {
            writer.writeCurrentToken(reader);
        }
    }
    if(reader.hasError()) {
        qCWarning(dcLgSmartTv) << "error reading XML device information:" << reader.errorString();
    }
    return xmlOut;
}

void TvDevice::eventOccured(const QByteArray &data)
{
    qCDebug(dcLgSmartTv) << "Event handler data received" << printXmlData(data);

    // if we got a channel changed event...
    if(data.contains("ChannelChanged")) {
        onChannelInformationUpdate(data);
        return;
    }

    //TODO: handle ip address change (dhcp) notification from the tv!

    // if the tv suspends, it will send a byebye message, which means
    // the pairing will be closed.
    if(data.contains("api type=\"pairing\"") && data.contains("byebye")) {
        qCDebug(dcLgSmartTv) << "Ended pairing (host)";
        setPaired(false);
        setReachable(false);
        return;
    }

    // check if this is a 3DMode changed event
    QXmlStreamReader xml(data);

    while(!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if(xml.name() == "name") {
            if(xml.readElementText() == "3DMode") {
                xml.readNext();
                if(xml.name() == "value") {
                    m_is3DMode = QVariant(xml.readElementText()).toBool();
                }
            }
        }
    }
    emit stateChanged();
}
