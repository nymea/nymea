#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent = 0);



private:
    QSettings* m_settings;

signals:
    void ipaddressChanged();
    void portChanged();

public slots:
    QString ipaddress();
    QString port();

    void setIPaddress(QString ipaddress);
    void setPort(QString port);

};

#endif // SETTINGS_H
