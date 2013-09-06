#include <QtGui/QGuiApplication>
#include "hiveclientcore.h"
#include "usb.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    HiveClientCore core;

    return app.exec();
}
