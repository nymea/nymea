#include "tvdiscovery.h"

TvDiscovery::TvDiscovery(QObject *parent) :
    QUdpSocket(parent)
{
    m_timeout = new QTimer(this);
    m_timeout->setSingleShot(true);
    connect(m_timeout,SIGNAL(timeout()),this,SLOT(discoverTimeout()));

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));

    m_port = 1900;
    m_host = QHostAddress("239.255.255.250");
    setSocketOption(QAbstractSocket::MulticastTtlOption,QVariant(1));
    setSocketOption(QAbstractSocket::MulticastLoopbackOption,QVariant(1));
    bind(QHostAddress::AnyIPv4,m_port,QUdpSocket::ShareAddress);
    joinMulticastGroup(m_host);
    connect(this,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error(QAbstractSocket::SocketError)));
    connect(this,SIGNAL(readyRead()),this,SLOT(readData()));
}

bool TvDiscovery::checkXmlData(QByteArray data)
{
    QByteArray xmlOut;
    QXmlStreamReader reader(data);
    QXmlStreamWriter writer(&xmlOut);
    writer.setAutoFormatting(true);

    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isWhitespace()) {
            writer.writeCurrentToken(reader);
        }
    }
    if(reader.hasError()){
        qDebug() << "ERROR reading XML device information:   " << reader.errorString();
        qDebug() << "--------------------------------------------";
        return false;
    }
    m_deviceInformationData = xmlOut;
    return true;
}

QString TvDiscovery::printXmlData(QByteArray data)
{
    QString xmlOut;
    QXmlStreamReader reader(data);
    QXmlStreamWriter writer(&xmlOut);
    writer.setAutoFormatting(true);

    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isWhitespace()) {
            writer.writeCurrentToken(reader);
        }
    }
    if(reader.hasError()){
        qDebug() << "ERROR reading XML device information:   " << reader.errorString();
        qDebug() << "--------------------------------------------";
    }
    return xmlOut;
}


void TvDiscovery::error(QAbstractSocket::SocketError error)
{
    qWarning() << errorString() << error;
}

void TvDiscovery::readData()
{
    QByteArray data;
    QHostAddress sender;
    quint16 udpPort;

    // read the answere from the multicast
    while (hasPendingDatagrams()) {
        data.resize(pendingDatagramSize());
        readDatagram(data.data(), data.size(), &sender, &udpPort);
    }

    if(data.size() > 0){
        if(data.contains("HTTP/1.1 200 OK")){
            const QStringList lines = QString(data).split("\r\n");

            QUrl location;
            QString uuid;
            QString server;

            foreach( const QString& line, lines){
                int separatorIndex = line.indexOf(':');
                QString key = line.left(separatorIndex).toUpper();
                QString value = line.mid(separatorIndex+1).trimmed();

                // get location
                if(key.contains("LOCATION")){
                    location = QUrl(value);
                }

                // get server info
                if(key.contains("SERVER")){
                    // check if it is a LG Smart Tv with UDAP/2.0 protocoll
                    if(value.contains("UDAP/2.0")){
                        server = value;
                    }
                }

                // get uuid
                if(key.contains("USN")){
                    int startIndex = value.indexOf(":");
                    int endIndex = value.indexOf("::");
                    uuid = value.mid(startIndex +1 ,(endIndex - startIndex)-1);
                }

                if(!location.isEmpty() && !uuid.isEmpty() && !server.isEmpty()){
                    foreach (TvDevice *device, m_tvList) {
                        if(device->uuid() == uuid){
                            return;
                        }
                    }
                    TvDevice *device = new TvDevice(this);
                    device->setLocation(location);
                    device->setHostAddress(sender);
                    device->setUuid(uuid);

                    //                    qDebug() << "--> UPnP searcher discovered a TV...";
                    //                    qDebug() << "location: " << location.toString();
                    //                    qDebug() << "ip: " << sender.toString();
                    //                    qDebug() << "uuid: " << uuid;
                    //                    qDebug() << "--------------------------------------------";
                    m_tvList.append(device);
                    requestDeviceInformation(device);
                }
            }
        }
    }
}

void TvDiscovery::discoverTimeout()
{
    emit discoveryDone(m_tvList);
}

void TvDiscovery::requestDeviceInformation(TvDevice *device)
{

    QNetworkRequest deviceRequest;
    deviceRequest.setUrl(device->location());
    deviceRequest.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("text/xml"));
    deviceRequest.setHeader(QNetworkRequest::UserAgentHeader,QVariant("UDAP/2.0"));

    m_deviceInformationReplay = m_manager->get(deviceRequest);
}

void TvDiscovery::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data;

    switch (status) {
    case(200):
        data = reply->readAll();
        if(checkXmlData(data)){
            parseDeviceInformation(data);
        }
        break;
    case(400):
        qDebug() << "ERROR: 400 Bad request. The event format is not valid or it has an incorrect value.";
        qDebug() << "--------------------------------------------";
        return;
    case(401):
        qDebug() << "ERROR: 401 Unauthorized. An event is sent when a Host and a Controller are not paired.";
        qDebug() << "--------------------------------------------";
        return;
    case(404):
        qDebug() << "ERROR: 404 Not Found. The POST path of an event is incorrect.";
        qDebug() << "--------------------------------------------";
        return;
    case(500):
        qDebug() << "ERROR: 500 Internal Server Error. Event Execution Failure.";
        qDebug() << "--------------------------------------------";
        return;
    default:
        return;
    }
}

void TvDiscovery::parseDeviceInformation(QByteArray data)
{

    QXmlStreamReader xml(data);

    QString name;
    QString uuid;
    QString modelName;
    QString deviceType;
    QString manufacturer;

    while(!xml.atEnd() && !xml.hasError()){
        xml.readNext();

        if(xml.isStartDocument()){
            continue;
        }
        if(xml.isStartElement()){
            if(xml.name() == "envelope"){
                continue;
            }
            //check if we have device part of message
            if(xml.name() == "device"){
                // seems to be device information
                while(!xml.atEnd()){
                    if(xml.name() == "deviceType" && xml.isStartElement()){
                        deviceType = xml.readElementText();
                    }
                    if(xml.name() == "modelName" && xml.isStartElement()){
                        modelName = xml.readElementText();
                    }
                    if(xml.name() == "friendlyName" && xml.isStartElement()){
                        name = xml.readElementText();
                    }
                    if(xml.name() == "manufacturer" && xml.isStartElement()){
                        manufacturer = xml.readElementText();
                    }
                    if(xml.name() == "uuid" && xml.isStartElement()){
                        uuid = xml.readElementText();
                    }
                    xml.readNext();
                }
            }
        }
    }

    foreach (TvDevice *device, m_tvList) {
        // find our device with this uuid
        if(device->uuid() == uuid){
            device->setName(name);
            device->setModelName(modelName);
            device->setDeviceType(deviceType);
            device->setManufacturer(manufacturer);

            qDebug() << "--> fetched TV information...";
            qDebug() << "name:          " << device->name();
            qDebug() << "model name:    " << device->modelName();
            qDebug() << "device type:   " << device->deviceType();
            qDebug() << "manufacturer:  " << device->manufacturer();
            qDebug() << "address:       " << device->hostAddress().toString();
            qDebug() << "location:      " << device->location().toString();
            qDebug() << "uuid:          " << device->uuid();
            qDebug() << "--------------------------------------------";
        }
    }
}

void TvDiscovery::discover(int timeout)
{
    QString searchMessage("M-SEARCH * HTTP/1.1\r\n"
                          "HOST:239.255.255.250:1900\r\n"
                          "MAN:\"ssdp:discover\"\r\n"
                          "MX:2\r\n"
                          "ST:udap:rootservice\r\n"
                          "USER-AGENT: UDAP/2.0 \r\n\r\n");

    m_tvList.clear();
    writeDatagram(searchMessage.toUtf8(),m_host,m_port);
    m_timeout->start(timeout);
}
