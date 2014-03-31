#include <QCoreApplication>
#include <guhcore.h>

#include <QtPlugin>

Q_IMPORT_PLUGIN(DevicePluginElro)
Q_IMPORT_PLUGIN(DevicePluginIntertechno)
Q_IMPORT_PLUGIN(DevicePluginMeisterAnker)
Q_IMPORT_PLUGIN(DevicePluginWifiDetector)
Q_IMPORT_PLUGIN(DevicePluginConrad)

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setOrganizationName("guhyourhome");

    GuhCore::instance();

    return a.exec();
}
