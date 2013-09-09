#ifndef RADIOSWITCH_H
#define RADIOSWITCH_H

#include <QObject>
#include <radioplugin/radioplugin.h>

#define RadioSwitchDelay 350

class RadioSwitch : public RadioPlugin
{
    Q_OBJECT
public:
    explicit RadioSwitch(QObject *parent = 0);

    QByteArray getBinCode();
    bool isValid(QList<int> rawData);

    enum RadioRemoteButton{
        A = 0x0,
        B = 0x1,
        C = 0x2,
        D = 0x3,
        E = 0x4
    };

private:
    int m_delay;
    QByteArray m_binCode;

signals:
    void switchSignalReceived(const QByteArray &channel, const char &button, const bool &buttonStatus);

public slots:
    QByteArray calcBinCode(const QByteArray &channel, const RadioRemoteButton &button, const bool &buttonStatus);

};

#endif // RADIOSWITCH_H
