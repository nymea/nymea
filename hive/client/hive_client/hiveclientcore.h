#ifndef HIVECLIENTCORE_H
#define HIVECLIENTCORE_H

#include <QObject>
#include <QQmlApplicationEngine>

#include "client.h"
#include "settings.h"

class HiveClientCore : public QObject
{
    Q_OBJECT
public:
    explicit HiveClientCore(QObject *parent = 0);
    
private:
    Client *m_client;
    //Settings *m_settings;

    QQmlApplicationEngine *m_engine;
    QObject *topLevel;

private slots:

signals:
    
public slots:
    
};

#endif // HIVECLIENTCORE_H
