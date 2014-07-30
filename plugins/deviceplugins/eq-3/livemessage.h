#ifndef LIVEMESSAGE_H
#define LIVEMESSAGE_H

#include <QObject>
#include <QDateTime>

class LiveMessage : public QObject
{
    Q_OBJECT
public:
    explicit LiveMessage(QObject *parent = 0);

    enum DeviceMode{
        Auto = 0,
        Manual = 1,
        Temporary = 2,
        Boost = 3
    };

    QByteArray rfAddress() const;
    void setRfAddress(const QByteArray & rfAddress);

    bool informationValid() const;
    void setInformationValid(const bool &informationValid);

    bool errorOccured() const;
    void setErrorOccured(const bool &errorOccured);

    bool isAnswereToCommand() const;
    void setIsAnswereToCommand(const bool &isAnswereToCommand);

    bool initialized() const;
    void setInitialized(const bool &initialized);

    bool batteryLow() const;
    void setBatteryLow(const bool &batteryLow);

    bool linkStatusOK() const;
    void setLinkStatusOK(const bool &linkStatusOK);

    bool panelLocked() const;
    void setPanelLocked(const bool &panelLocked);

    bool gatewayKnown() const;
    void setGatewayKnown(const bool &gatewayKnown);

    bool dtsActive() const;
    void setDtsActive(const bool &dtsActive);

    int deviceMode() const;
    void setDeviceMode(const int &deviceMode);

    QString deviceModeString() const;

    int valvePosition() const;
    void setValvePosition(const int &valvePosition);

    double setpointTemperature() const;
    void setSetpointTemperatre(const double &setpointTemperature);

    QDateTime dateTime() const;
    void setDateTime(const QDateTime dateTime);


private:

    QByteArray m_rfAddress;
    bool m_informationValid;
    bool m_errorOccured;
    bool m_isAnswerToCommand;
    bool m_initialized;
    bool m_batteryLow;
    bool m_linkStatusOK;
    bool m_panelLocked;
    bool m_gatewayKnown;
    bool m_dtsActive;
    int m_deviceMode;
    QString m_deviceModeString;
    int m_valvePosition;
    double m_setpointTemperature;
    QDateTime m_dateTime;

signals:

public slots:

};

#endif // LIVEMESSAGE_H
