#ifndef TVDEVICE_H
#define TVDEVICE_H

#include <QObject>
#include <QUrl>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>


class TvDevice : public QObject
{
    Q_OBJECT
public:
    explicit TvDevice(QObject *parent = 0);

    void setLocation(const QUrl &location);
    QUrl location() const;

    void setHostAddress(const QHostAddress &hostAddress);
    QHostAddress hostAddress() const;

    void setName(const QString &name);
    QString name() const;

    void setModelName(const QString &modelName);
    QString modelName() const;

    void setManufacturer(const QString &manufacturer);
    QString manufacturer() const;

    void setDeviceType(const QString &deviceType);
    QString deviceType() const;

    void setUuid(const QString &uuid);
    QString uuid() const;

    void setKey(const QString &key);
    QString key() const;

    void showPairingKey();



private:
    QUrl m_location;
    QHostAddress m_hostAddress;
    QString m_name;
    QString m_modelName;
    QString m_manufacturer;
    QString m_deviceType;
    QString m_uuid;
    QString m_key;
    bool m_pairingStatus;

    QNetworkAccessManager *m_manager;
    QNetworkReply *m_showKeyReplay;


signals:

public slots:

};

#endif // TVDEVICE_H
