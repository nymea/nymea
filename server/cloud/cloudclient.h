#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include <QObject>

class CloudClient : public QObject
{
    Q_OBJECT
public:
    explicit CloudClient(QObject *parent = 0);

signals:

public slots:
};

#endif // CLOUDCLIENT_H
