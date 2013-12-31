#include <QCoreApplication>
#include <hivecore.h>

#include <QtPlugin>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setOrganizationName("hiveyourhome");

    HiveCore::instance();

    return a.exec();
}
