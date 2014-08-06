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

#include "plugin/deviceplugin.h"

class TvDevice : public QObject
{
    Q_OBJECT
public:
    explicit TvDevice(QObject *parent = 0);

    enum RemoteKey{
        Power           = 1,
        Num0            = 2,
        Num1            = 3,
        Num2            = 4,
        Num3            = 5,
        Num4            = 6,
        Num5            = 7,
        Num6            = 8,
        Num7            = 9,
        Num8            = 10,
        Num9            = 11,
        Up              = 12,
        Down            = 13,
        Left            = 14,
        Right           = 15,
        Ok              = 20,
        Home            = 21,
        Menu            = 22,
        Back            = 23,
        VolUp           = 24,
        VolDown         = 25,
        Mute            = 26,
        ChannelUp       = 27,
        ChannelDown     = 28,
        Blue            = 29,
        Green           = 30,
        Red             = 31,
        Yellow          = 32,
        Play            = 33,
        Pause           = 34,
        Stop            = 35,
        FastForward     = 36,
        Rewind          = 37,
        SkipForward     = 38,
        SkipBackward    = 39,
        Record          = 40,
        RecrodList      = 41,
        Repeat          = 42,
        LiveTv          = 43,
        EPG             = 44,
        Info            = 45,
        AspectRatio     = 46,
        ExternalInput   = 47,
        PIP             = 48,
        ChangeSub       = 49,
        ProgramList     = 50,
        TeleText        = 51,
        Mark            = 52,
        Video3D         = 400,
        LR_3D           = 401,
        Dash            = 402,
        PrevoiusChannel = 403,
        FavouritChannel = 404,
        QuickMenu       = 405,
        TextOption      = 406,
        AudioDescription= 407,
        NetCastKey      = 408,
        EnergySaving    = 409,
        AV_Mode         = 410,
        SimpLink        = 411,
        Exit            = 412,
        ReservationList = 413,
        PipUp           = 414,
        PipDown         = 415,
        SwitchVideo     = 416,
        MyApps          = 417
    };

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

    bool reachable() const;
    bool paired() const;

    void showPairingKey();
    void requestPairing();
    void sendCommand(TvDevice::RemoteKey key, ActionId actionId);

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
    bool m_reachable;

    ActionId m_actionId;

    QNetworkAccessManager *m_manager;
    QNetworkReply *m_showKeyReplay;
    QNetworkReply *m_requestPairingReplay;
    QNetworkReply *m_finishingPairingReplay;
    QNetworkReply *m_sendCommandReplay;

    void finishingPairing();

signals:
    void pairingFinished(const bool &success);
    void statusChanged();
    void sendCommandFinished(const bool &succeeded, const ActionId &actionId);

private slots:
    void replyFinished(QNetworkReply *reply);


public slots:

};

#endif // TVDEVICE_H
