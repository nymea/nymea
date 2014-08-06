#include "tvdevice.h"

TvDevice::TvDevice(QObject *parent) :
    QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);

    m_key = "539887";
    m_pairingStatus = false;
    m_reachable = false;

    connect(m_manager, &QNetworkAccessManager::finished, this, &TvDevice::replyFinished);
}

void TvDevice::setLocation(const QUrl &location)
{
    m_location = location;
}

QUrl TvDevice::location() const
{
    return m_location;
}

void TvDevice::setHostAddress(const QHostAddress &hostAddress)
{
    m_hostAddress = hostAddress;
}

QHostAddress TvDevice::hostAddress() const
{
    return m_hostAddress;
}

void TvDevice::setName(const QString &name)
{
    m_name = name;
}

QString TvDevice::name() const
{
    return m_name;
}

void TvDevice::setModelName(const QString &modelName)
{
    m_modelName = modelName;
}

QString TvDevice::modelName() const
{
    return m_modelName;
}

void TvDevice::setManufacturer(const QString &manufacturer)
{
    m_manufacturer = manufacturer;
}

QString TvDevice::manufacturer() const
{
    return m_manufacturer;
}

void TvDevice::setDeviceType(const QString &deviceType)
{
    m_deviceType = deviceType;
}

QString TvDevice::deviceType() const
{
    return m_deviceType;
}

void TvDevice::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString TvDevice::uuid() const
{
    return m_uuid;
}

void TvDevice::setKey(const QString &key)
{
    m_key = key;
}

QString TvDevice::key() const
{
    return m_key;
}

bool TvDevice::reachable() const
{
    return m_reachable;
}

bool TvDevice::paired() const
{
    return m_pairingStatus;
}

void TvDevice::showPairingKey()
{
    qDebug() << "request show pairing key on screen...";
    qDebug() << "--------------------------------------------";

    QString urlString = "http://" + m_hostAddress.toString() + ":8080/udap/api/pairing";

    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0 guh"));

    QByteArray data = "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"pairing\"> <name>showKey</name></api></envelope>";

    m_showKeyReplay = m_manager->post(request,data);
}

void TvDevice::requestPairing()
{
    if(m_key.isNull()){
        emit pairingFinished(false);
    }

    QString urlString = "http://" + m_hostAddress.toString()  +":8080/udap/api/pairing";

    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0 guh"));

    QByteArray data = "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"pairing\"><name>hello</name><value>" + m_key.toUtf8() + "</value><port>8080</port></api></envelope>";

    m_requestPairingReplay = m_manager->post(request,data);
}

void TvDevice::sendCommand(TvDevice::RemoteKey key, ActionId actionId)
{
    m_actionId = actionId;

    if(!m_pairingStatus){
        requestPairing();
        return;
    }

    QString urlString = "http://" + m_hostAddress.toString()  +":8080/udap/api/command";

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

void TvDevice::finishingPairing()
{
    QString urlString = "http://" + m_hostAddress.toString()  +":8080/udap/api/pairing";

    QNetworkRequest request;
    request.setUrl(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    request.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0 guh"));

    QByteArray data = "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"pairing\"><name>byebye</name><port>8080</port></api></envelope>";

    m_finishingPairingReplay = m_manager->post(request,data);
}

void TvDevice::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(status != 200){
        m_reachable = false;
    }else{
        m_reachable = true;
    }

    if(reply == m_showKeyReplay){
        if(status != 200){
            qWarning() << "ERROR: could not request to show pairing key on screen " << status;
        }
    }
    if(reply == m_requestPairingReplay){
        if(status != 200){
            m_pairingStatus = false;
            emit pairingFinished(false);
            qWarning() << "ERROR: could not pair with device" << status;
        }else{
            m_pairingStatus = true;
            qDebug() << "successfully paired with tv " << m_modelName;
            emit pairingFinished(true);        }
    }

    if(reply == m_finishingPairingReplay){
        if(status == 200){
            m_pairingStatus = false;
            qDebug() << "successfully unpaired from tv " << m_modelName;
        }
    }

    if(reply == m_sendCommandReplay){
        if(status != 200){
            emit sendCommandFinished(false,m_actionId);
            qWarning() << "ERROR: could not send comand" << status;
        }else{
            m_pairingStatus = true;
            qDebug() << "successfully sent command to tv " << m_modelName;
            emit sendCommandFinished(true,m_actionId);
        }
    }
    emit statusChanged();
}
