#include <QCoreApplication>
#include <hivecore.h>

#include <QtPlugin>

Q_IMPORT_PLUGIN(DevicePluginElro)
Q_IMPORT_PLUGIN(DevicePluginIntertechno)
Q_IMPORT_PLUGIN(DevicePluginMeisterAnker)
Q_IMPORT_PLUGIN(DevicePluginWifiDetector)

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setOrganizationName("hiveyourhome");

    HiveCore::instance();

    return a.exec();
}
