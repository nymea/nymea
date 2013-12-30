#include <QCoreApplication>
#include <hivecore.h>

#include <QtPlugin>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    HiveCore::instance();

    return a.exec();
}
