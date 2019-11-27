#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include <QObject>
#include <QUuid>
#include <QQmlParserStatus>

#include "types/event.h"
#include "devices/devicemanager.h"

namespace nymeaserver {

class ScriptEvent: public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString eventTypeId READ eventTypeId WRITE setEventTypeId NOTIFY eventTypeIdChanged)
    Q_PROPERTY(QString eventName READ eventName WRITE setEventName NOTIFY eventNameChanged)
public:
    ScriptEvent(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString deviceId() const;
    void setDeviceId(const QString &deviceId);

    QString eventTypeId() const;
    void setEventTypeId(const QString &eventTypeId);

    QString eventName() const;
    void setEventName(const QString &eventName);

private slots:
    void onEventTriggered(const Event &event);

signals:
    void deviceIdChanged();
    void eventTypeIdChanged();
    void eventNameChanged();

    void triggered();

private:
    DeviceManager *m_deviceManager = nullptr;

    QString m_deviceId;
    QString m_eventTypeId;
    QString m_eventName;
};

}

#endif // EVENTLISTENER_H
