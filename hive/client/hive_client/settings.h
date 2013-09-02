#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString ipaddress READ ipaddress WRITE setIPaddress NOTIFY ipaddressChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

public:
    explicit Settings(QObject *parent = 0);

    QString ipaddress() const;
    void setIPaddress(QString ipaddress);

    int port() const;
    void setPort(int port);

private:

signals:
    void ipaddressChanged();
    void portChanged();

public slots:


};

#endif // SETTINGS_H
