#include "hiveclientcore.h"
#include <QtQml>
#include <QQmlEngine>
#include <QQuickWindow>

HiveClientCore::HiveClientCore(QObject *parent) :
    QObject(parent)
{

    m_client = new Client(this);


    // QML Typs
    qmlRegisterType<Settings>("hive",1,0,"Settings");

    // QML application window
    m_engine = new QQmlApplicationEngine(this);
    m_engine->load(QUrl("qml/hive_client/main.qml"));

    m_engine->rootContext()->setContextProperty("client", m_client);

    topLevel = m_engine->rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    window->show();


}

