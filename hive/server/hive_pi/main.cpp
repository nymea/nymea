#include <QCoreApplication>
#include <hivecore.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    HiveCore core;

    return a.exec();
}
