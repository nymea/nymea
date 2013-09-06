#ifndef RFSWITCH_H
#define RFSWITCH_H

#include <QObject>
#include <radio/plugins/radioplugin.h>

#define RFSwitchDelay 320

class RFSwitch : public RadioPlugin
{
public:
    explicit RFSwitch(QObject *parent = 0);

    QByteArray getBinCode();
    bool isValid(QList<int> rawData);

private:
    int m_delay;
    QByteArray m_binCode;

signals:
    void switchSignalReceived(const QByteArray &channel, const char &button, const bool &buttonStatus);

};

#endif // RFSWITCH_H
