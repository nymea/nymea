#include "tvdevice.h"

TvDevice::TvDevice(QObject *parent) :
    QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);



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

void TvDevice::showPairingKey()
{
    qDebug() << "request show pairing key on screen...";
    qDebug() << "--------------------------------------------";

    QString urlString = "http://" + m_hostAddress.toString() + ":8080/udap/api/pairing";

    QNetworkRequest pairingRequest;
    pairingRequest.setUrl(QUrl(urlString));
    pairingRequest.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml; charset=utf-8"));
    pairingRequest.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0"));

    QByteArray data = "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><api type=\"pairing\"> <name>showKey</name></api></envelope>";

    m_showKeyReplay = m_manager->post(pairingRequest,data);
}



