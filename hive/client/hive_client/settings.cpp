#include "settings.h"
#include <QDebug>

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    m_settings = new QSettings(QString(),"hiveClient",this);
}

QString Settings::ipaddress()
{
    return m_settings->value("server","10.10.10.40").toString();
}

void Settings::setIPaddress(QString ipaddress)
{
    m_settings->setValue("server",ipaddress);
    emit ipaddressChanged();
}

QString Settings::port()
{
    return m_settings->value("port","1234").toString();
}

void Settings::setPort(QString port)
{
    m_settings->setValue("port",port);
    emit ipaddressChanged();
}

