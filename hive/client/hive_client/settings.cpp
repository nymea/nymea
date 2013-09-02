#include "settings.h"
#include <QDebug>

Settings::Settings(QObject *parent) :
    QObject(parent)
{
}

QString Settings::ipaddress() const
{
    QSettings settings("hive");
    return settings.value("server","10.10.10.40").toString();
}

void Settings::setIPaddress(QString ipaddress)
{
    QSettings settings("hive");
    settings.setValue("server",ipaddress);
    emit ipaddressChanged();
}

int Settings::port() const
{
    QSettings settings("hive");
    return settings.value("port","1234").toInt();
}

void Settings::setPort(int port)
{
    QSettings settings("hive");
    settings.setValue("port",port);
    emit ipaddressChanged();
}

