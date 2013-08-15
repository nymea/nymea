#ifndef HIVECORE_H
#define HIVECORE_H

#include <QObject>
#include "server.h"

class HiveCore : public QObject
{
    Q_OBJECT
public:
    explicit HiveCore(QObject *parent = 0);
    
private:
    Server *m_server;

signals:
    
public slots:
    
};

#endif // HIVECORE_H
