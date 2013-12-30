#ifndef RADIO433_H
#define RADIO433_h

#include <QObject>

class Radio433: public QObject
{
    Q_OBJECT

public:
    Radio433(QObject *parent = 0);

public:
    void sendData(QList<int> rawData);

signals:
    void dataReceived(QList<int> rawData);
};

#endif
