#ifndef MAXCUBEDISCOVERY_H
#define MAXCUBEDISCOVERY_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>

#include "maxcube.h"

class MaxCubeDiscovery : public QObject
{
    Q_OBJECT
public:
    explicit MaxCubeDiscovery(QObject *parent = 0);

    void detectCubes();

private:
    QUdpSocket *m_udpSocket;
    QTimer *m_timeout;

    quint16 m_port;

    QList<MaxCube*> m_cubeList;

private slots:
    void readData();
    void discoverTimeout();

signals:
    void cubesDetected(const QList<MaxCube*> &cubeList);

public slots:

};

#endif // MAXCUBEDISCOVERY_H
