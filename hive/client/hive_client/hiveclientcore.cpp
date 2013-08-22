#include "hiveclientcore.h"
#include <QtQml>
#include <QQmlEngine>
#include <QQmlApplicationEngine>

HiveClientCore::HiveClientCore(QObject *parent) :
    QObject(parent)
{

    m_client = new Client(this);
    m_client->connectToHost("10.10.10.40","1234");

    m_viewer = new QtQuick2ApplicationViewer;
    m_viewer->setMainQmlFile(QStringLiteral("qml/hive_client/main.qml"));
    m_viewer->showExpanded();
}
