#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H

#include <QObject>

class NetworkConnection : public QObject
{
    Q_OBJECT
public:
    explicit NetworkConnection(QObject *parent = 0);

signals:

public slots:
};

#endif // NETWORKCONNECTION_H