#include "maxcubediscovery.h"

MaxCubeDiscovery::MaxCubeDiscovery(QObject *parent) :
    QObject(parent)
{
    // UDP broadcast for cube detection in the network
    m_udpSocket = new QUdpSocket(this);
    m_port = 23272;
    m_udpSocket->bind(m_port,QUdpSocket::ShareAddress);

    m_timeout = new QTimer(this);
    m_timeout->setSingleShot(true);

    connect(m_udpSocket,SIGNAL(readyRead()),this,SLOT(readData()));
    connect(m_timeout,SIGNAL(timeout()),this,SLOT(discoverTimeout()));
}

void MaxCubeDiscovery::detectCubes()
{
    qDebug() << "====================================================";
    qDebug() << "       searching for cubes....";

    m_cubeList.clear();

    // broadcast the hello message, every cube should respond with a 26 byte message
    m_udpSocket->writeDatagram("eQ3Max*.**********I", QHostAddress::Broadcast, m_port);

    m_timeout->start(1500);
}

void MaxCubeDiscovery::readData()
{
    QByteArray data;
    QHostAddress sender;
    quint16 udpPort;

    // read the answere from the
    while (m_udpSocket->hasPendingDatagrams()) {
        data.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(data.data(), data.size(), &sender, &udpPort);
    }
    if(!data.isEmpty() && data.contains("eQ3MaxAp")){

        QString serialNumber = data.mid(8,10);
        QByteArray rfAddress = data.mid(21,3).toHex();
        int firmware = data.mid(24,2).toHex().toInt();
        qint16 port;
        // set port depending on the firmware
        if(firmware < 109){
            port= 80;
        }else{
            port = 62910;
        }

        MaxCube *cube = new MaxCube(this, serialNumber, sender, port);
        cube->setRfAddress(rfAddress);
        qDebug() << "====================================================";
        qDebug() << "       cube detected...";
        qDebug() << "====================================================";
        qDebug() << "           serial number | " << cube->serialNumber();
        qDebug() << "            host address | " << cube->hostAddress().toString();
        qDebug() << "                    port | " << QString::number(cube->port());
        qDebug() << "              rf address | " << cube->rfAddress();
        qDebug() << "                firmware | " << QString::number(cube->firmware());
        qDebug() << "====================================================";

        m_cubeList.append(cube);
    }
}

void MaxCubeDiscovery::discoverTimeout()
{
    emit cubesDetected(m_cubeList);
}
