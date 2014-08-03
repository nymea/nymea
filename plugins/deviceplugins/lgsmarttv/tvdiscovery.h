#ifndef TVDISCOVERY_H
#define TVDISCOVERY_H

#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>

#include "tvdevice.h"

class TvDiscovery : public QUdpSocket
{
    Q_OBJECT
public:
    explicit TvDiscovery(QObject *parent = 0);

private:
    QHostAddress m_host;
    qint16 m_port;

    QTimer *m_timeout;
    QList<TvDevice*> m_tvList;

    QNetworkAccessManager *m_manager;
    QNetworkReply *m_deviceInformationReplay;

    QByteArray m_deviceInformationData;
    bool checkXmlData(QByteArray data);
    QString printXmlData(QByteArray data);

signals:
    void discoveryDone(const QList<TvDevice*> deviceList);

private slots:
    void error(QAbstractSocket::SocketError error);
    void readData();
    void discoverTimeout();

    void requestDeviceInformation(TvDevice *device);
    void replyFinished(QNetworkReply *reply);
    void parseDeviceInformation(QByteArray data);

public slots:
    void discover(int timeout);

};

#endif // TVDISCOVERY_H
