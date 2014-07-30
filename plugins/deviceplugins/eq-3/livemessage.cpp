#include "livemessage.h"

LiveMessage::LiveMessage(QObject *parent) :
    QObject(parent)
{
}

QByteArray LiveMessage::rfAddress() const
{
    return m_rfAddress;
}

void LiveMessage::setRfAddress(const QByteArray &rfAddress)
{
    m_rfAddress = rfAddress;
}

bool LiveMessage::informationValid() const
{
    return m_informationValid;
}

void LiveMessage::setInformationValid(const bool &informationValid)
{
    m_informationValid = informationValid;
}

bool LiveMessage::errorOccured() const
{
    return m_errorOccured;
}

void LiveMessage::setErrorOccured(const bool &errorOccured)
{
    m_errorOccured = errorOccured;
}

bool LiveMessage::isAnswereToCommand() const
{
    return m_isAnswerToCommand;
}

void LiveMessage::setIsAnswereToCommand(const bool &isAnswereToCommand)
{
    m_isAnswerToCommand = isAnswereToCommand;
}

bool LiveMessage::initialized() const
{
    return m_initialized;
}

void LiveMessage::setInitialized(const bool &initialized)
{
    m_initialized = initialized;
}

bool LiveMessage::batteryLow() const
{
    return m_batteryLow;
}

void LiveMessage::setBatteryLow(const bool &batteryLow)
{
    m_batteryLow = batteryLow;
}

bool LiveMessage::linkStatusOK() const
{
    return m_linkStatusOK;
}

void LiveMessage::setLinkStatusOK(const bool &linkStatusOK)
{
    m_linkStatusOK = linkStatusOK;
}

bool LiveMessage::panelLocked() const
{
    return m_panelLocked;
}

void LiveMessage::setPanelLocked(const bool &panelLocked)
{
    m_panelLocked = panelLocked;
}

bool LiveMessage::gatewayKnown() const
{
    return m_gatewayKnown;
}

void LiveMessage::setGatewayKnown(const bool &gatewayKnown)
{
    m_gatewayKnown = gatewayKnown;
}

bool LiveMessage::dtsActive() const
{
    return m_dtsActive;
}

void LiveMessage::setDtsActive(const bool &dtsActive)
{
    m_dtsActive = dtsActive;
}

int LiveMessage::deviceMode() const
{
    return m_deviceMode;
}

void LiveMessage::setDeviceMode(const int &deviceMode)
{
    m_deviceMode = deviceMode;

    switch (deviceMode) {
    case Auto:
        m_deviceModeString = "Auto";
        break;
    case Manual:
        m_deviceModeString = "Manuel";
        break;
    case Temporary:
        m_deviceModeString = "Temporary";
        break;
    case Boost:
        m_deviceModeString = "Boost";
        break;
    default:
        m_deviceModeString = "-";
        break;
    }

}

QString LiveMessage::deviceModeString() const
{
    return m_deviceModeString;
}

int LiveMessage::valvePosition() const
{
    return m_valvePosition;
}

void LiveMessage::setValvePosition(const int &valvePosition)
{
    m_valvePosition = valvePosition;
}

double LiveMessage::setpointTemperature() const
{
    return m_setpointTemperature;
}

void LiveMessage::setSetpointTemperatre(const double &setpointTemperature)
{
    m_setpointTemperature = setpointTemperature;
}

QDateTime LiveMessage::dateTime() const
{
    return m_dateTime;
}

void LiveMessage::setDateTime(const QDateTime dateTime)
{
    m_dateTime = dateTime;
}
