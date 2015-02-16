/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
#include <QXmlReader>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>

#include "plugin/deviceplugin.h"
#include "tveventhandler.h"
#include "network/upnpdiscovery/upnpdevice.h"

class TvDevice : public UpnpDevice
{
    Q_OBJECT
public:
    explicit TvDevice(QObject *parent = 0, UpnpDeviceDescriptor upnpDeviceDescriptor = UpnpDeviceDescriptor());

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

    // propertys
    void setKey(const QString &key);
    QString key() const;

    bool paired() const;

    // States
    bool isReachable() const;
    bool is3DMode() const;
    int volumeLevel() const;
    bool mute() const;
    QString channelType() const;
    QString channelName() const;
    int channelNumber() const;
    QString programName() const;
    int inputSourceIndex() const;
    QString inputSourceLabelName() const;

    // public actions
    void showPairingKey();
    void requestPairing();
    void endPairing();
    void sendCommand(TvDevice::RemoteKey key, ActionId actionId);
    void setupEventHandler();
    void refresh();

private:
    QString m_key;
    bool m_pairingStatus;

    // States
    bool m_is3DMode;
    bool m_reachable;
    int m_volumeLevel;
    bool m_mute;
    QString m_channelType;
    QString m_channelName;
    int m_channelNumber;
    QString m_programName;
    int m_inputSourceIndex;
    QString m_inputSourceLabel;

    ActionId m_actionId;

    QNetworkAccessManager *m_manager;
    QNetworkReply *m_showKeyReplay;
    QNetworkReply *m_requestPairingReplay;
    QNetworkReply *m_finishingPairingReplay;
    QNetworkReply *m_sendCommandReplay;
    QNetworkReply *m_queryVolumeInformationReplay;
    QNetworkReply *m_queryChannelInformationReplay;

    TvEventHandler *m_eventHandler;

    QString printXmlData(QByteArray data);
    void queryVolumeInformation();
    void queryChannelInformation();
    void parseVolumeInformation(const QByteArray &data);
    void parseChannelInformation(const QByteArray &data);

signals:
    void pairingFinished(const bool &success);
    void statusChanged();
    void sendCommandFinished(const bool &succeeded, const ActionId &actionId);

private slots:
    void replyFinished(QNetworkReply *reply);
    void eventOccured(const QByteArray &data);

};

#endif // TVDEVICE_H
