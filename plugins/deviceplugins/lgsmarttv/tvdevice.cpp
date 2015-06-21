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
#include "loggingcategories.h"

TvDevice::TvDevice(QObject *parent, UpnpDeviceDescriptor upnpDeviceDescriptor) :
    UpnpDevice(parent, upnpDeviceDescriptor)
{
    m_manager = new QNetworkAccessManager(this);

    m_key = "0";
    m_pairingStatus = false;
    m_reachable = false;

    connect(m_manager, &QNetworkAccessManager::finished, this, &TvDevice::replyFinished);
}

void TvDevice::setKey(const QString &key)
{
    m_key = key;
}

QString TvDevice::key() const
{
    return m_key;
}

bool TvDevice::isReachable() const
{
    return m_reachable;
}

bool TvDevice::paired() const
{
    return m_pairingStatus;
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

void TvDevice::showPairingKey()
{
    QString urlString = "http://" + hostAddress().toString() + ":" + QString::number(port()) + "/udap/api/pairing";

    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0"));

    QByteArray data = "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"pairing\"> <name>showKey</name></api></envelope>";

    m_showKeyReplay = m_manager->post(request,data);
}

void TvDevice::requestPairing()
{
    if(m_key.isNull()){
        emit pairingFinished(false);
    }

    QString urlString = "http://" + hostAddress().toString()  + ":" + QString::number(port()) + "/udap/api/pairing";

    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0 guh"));

    QByteArray data = "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"pairing\"><name>hello</name><value>" + m_key.toUtf8() + "</value><port>8080</port></api></envelope>";

    m_requestPairingReplay = m_manager->post(request,data);
}

void TvDevice::endPairing()
{
    QString urlString = "http://" + hostAddress().toString()  + ":" + QString::number(port()) + "/udap/api/pairing";

    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0 guh"));
    request.setRawHeader("Connection", "Close");

    QByteArray data = "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"pairing\"><name>byebye</name><port>8080</port></api></envelope>";

    m_finishingPairingReplay = m_manager->post(request,data);
}


void TvDevice::sendCommand(TvDevice::RemoteKey key, ActionId actionId)
{
    m_actionId = actionId;

    if(!m_pairingStatus) {
        requestPairing();
        return;
    }

    QString urlString = "http://" + hostAddress().toString()  + ":" + QString::number(port()) + "/udap/api/command";

    QByteArray data;
    data.append("<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"command\"><name>HandleKeyInput</name><value>");
    data.append(QString::number(key).toUtf8());
    data.append("</value></api></envelope>");

    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0 guh"));

    m_sendCommandReplay = m_manager->post(request,data);
}

void TvDevice::setupEventHandler()
{
    qCDebug(dcLgSmartTv) << "set up event handler " << hostAddress().toString() << port();
    m_eventHandler = new TvEventHandler(this, hostAddress(), port());
    connect(m_eventHandler, &TvEventHandler::eventOccured, this, &TvDevice::eventOccured);
}

void TvDevice::refresh()
{
    if(paired()) {
        queryChannelInformation();
        queryVolumeInformation();
    }else{
        requestPairing();
    }
}

void TvDevice::queryVolumeInformation()
{
    QString urlString = "http://" + hostAddress().toString()  + ":" + QString::number(port()) + "/udap/api/data?target=volume_info";

    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0"));
    request.setRawHeader("Connection", "Close");

    m_queryVolumeInformationReplay = m_manager->get(request);
}

void TvDevice::queryChannelInformation()
{
    QString urlString = "http://" + hostAddress().toString()  + ":" + QString::number(port()) + "/udap/api/data?target=cur_channel";

    QNetworkRequest deviceRequest;
    deviceRequest.setUrl(QUrl(urlString));
    deviceRequest.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml"));
    deviceRequest.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0"));
    deviceRequest.setRawHeader("Connection", "Close");

    m_queryChannelInformationReplay = m_manager->get(deviceRequest);
}

void TvDevice::parseVolumeInformation(const QByteArray &data)
{
    qCDebug(dcLgSmartTv) << printXmlData(data);
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
    emit statusChanged();
}

void TvDevice::parseChannelInformation(const QByteArray &data)
{
    qCDebug(dcLgSmartTv) << printXmlData(data);
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
    emit statusChanged();
}

QString TvDevice::printXmlData(QByteArray data)
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
        qCWarning(dcLgSmartTv) << "error reading XML device information:   " << reader.errorString();
    }
    return xmlOut;
}

void TvDevice::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(status != 200)  {
        m_reachable = false;
    } else {
        m_reachable = true;
    }

    if(reply == m_showKeyReplay) {
        if(status != 200) {
           qCWarning(dcLgSmartTv) << "ERROR: could not request to show pairing key on screen " << status;
        }
        m_showKeyReplay->deleteLater();
    }
    if(reply == m_requestPairingReplay) {
        if(status != 200) {
            m_pairingStatus = false;
            emit pairingFinished(false);
            qCWarning(dcLgSmartTv) << "could not pair with device" << status;
        } else {
            m_pairingStatus = true;
            qCDebug(dcLgSmartTv) << "successfully paired with tv " << modelName();
            emit pairingFinished(true);
        }
        m_requestPairingReplay->deleteLater();
    }

    if(reply == m_finishingPairingReplay) {
        if(status == 200) {
            m_pairingStatus = false;
            qCDebug(dcLgSmartTv) << "successfully unpaired from tv " << modelName();
        }
        m_finishingPairingReplay->deleteLater();
    }

    if(reply == m_sendCommandReplay) {
        if (status != 200) {
            emit sendCommandFinished(false,m_actionId);
            qCWarning(dcLgSmartTv) << "ERROR: could not send comand" << status;
        } else {
            m_pairingStatus = true;
            qCDebug(dcLgSmartTv) << "successfully sent command to tv " << modelName();
            emit sendCommandFinished(true,m_actionId);
            refresh();
        }
        m_sendCommandReplay->deleteLater();
    }
    if(reply == m_queryVolumeInformationReplay) {
        parseVolumeInformation(reply->readAll());
        m_queryVolumeInformationReplay->deleteLater();
    }
    if(reply == m_queryChannelInformationReplay) {
        parseChannelInformation(reply->readAll());
        m_queryChannelInformationReplay->deleteLater();
    }

    emit statusChanged();
}

void TvDevice::eventOccured(const QByteArray &data)
{
    // if we got a channel changed event...
    if(data.contains("ChannelChanged")) {
        parseChannelInformation(data);
        return;
    }

    // if the tv suspends, it will send a byebye message, which means
    // the pairing will be closed.
    if(data.contains("api type=\"pairing\"") && data.contains("byebye")) {
        qCDebug(dcLgSmartTv) << "--> tv ended pairing";
        m_pairingStatus = false;
        m_reachable = false;
        emit statusChanged();
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

    emit statusChanged();
}
