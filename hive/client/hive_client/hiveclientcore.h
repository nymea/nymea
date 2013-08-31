#ifndef HIVECLIENTCORE_H
#define HIVECLIENTCORE_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "client.h"
#include "settings.h"

class HiveClientCore : public QObject
{
    Q_OBJECT
public:
    explicit HiveClientCore(QObject *parent = 0);
    
private:
    Client *m_client;
    Settings *m_settings;

    QQmlApplicationEngine *m_engine;
    QObject *topLevel;

    int m_id;

private slots:
    void onConnected();
    void sendSomething(QString deviceName, QString method);

signals:
    
public slots:
    
};

#endif // HIVECLIENTCORE_H
