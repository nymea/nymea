#include <QtGui/QGuiApplication>
#include "qtquick2applicationviewer.h"
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    //QQmlApplicationEngine engine("qml/hive_client/main.qml");


    QtQuick2ApplicationViewer viewer;
    viewer.setMainQmlFile(QStringLiteral("qml/hive_client/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
