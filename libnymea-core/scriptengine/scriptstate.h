#ifndef SCRIPTSTATE_H
#define SCRIPTSTATE_H

#include <QObject>
#include <QQmlEngine>
#include <QPointer>

#include "devices/devicemanager.h"
#include "devices/deviceactioninfo.h"

namespace nymeaserver {

class ScriptState : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString stateTypeId READ stateTypeId WRITE setStateTypeId NOTIFY stateTypeIdChanged)
    Q_PROPERTY(QString stateName READ stateName WRITE setStateName NOTIFY stateNameChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit ScriptState(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString deviceId() const;
    void setDeviceId(const QString &deviceId);

    QString stateTypeId() const;
    void setStateTypeId(const QString &stateTypeId);

    QVariant value() const;
    void setValue(const QVariant &value);

public slots:
    void store();
    void restore();

signals:
    void deviceIdChanged();
    void stateTypeIdChanged();
    void stateNameChanged();
    void valueChanged();

private slots:
    void onDeviceStateChanged(Device *device, const StateTypeId &stateTypeId);

private:
    DeviceManager *m_deviceManager = nullptr;

    QString m_deviceId;
    QString m_stateTypeId;

    DeviceActionInfo *m_pendingActionInfo = nullptr;
    QVariant m_valueCache;

    QVariant m_valueStore;
};

}

#endif // SCRIPTSTATE_H
