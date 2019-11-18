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
    Q_PROPERTY(QString stateTypeId READ stateTypeId WRITE setStateTypeId NOTIFY stateTypeChanged)
    Q_PROPERTY(QString stateName READ stateName WRITE setStateName NOTIFY stateTypeChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QVariant minimumValue READ minimumValue NOTIFY stateTypeChanged)
    Q_PROPERTY(QVariant maximumValue READ maximumValue NOTIFY stateTypeChanged)

public:
    explicit ScriptState(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString deviceId() const;
    void setDeviceId(const QString &deviceId);

    QString stateTypeId() const;
    void setStateTypeId(const QString &stateTypeId);

    QString stateName() const;
    void setStateName(const QString &stateName);

    QVariant value() const;
    void setValue(const QVariant &value);

    QVariant minimumValue() const;
    QVariant maximumValue() const;

public slots:
    void store();
    void restore();

signals:
    void deviceIdChanged();
    void stateTypeChanged();
    void valueChanged();

private slots:
    void onDeviceStateChanged(Device *device, const StateTypeId &stateTypeId);

private:
    DeviceManager *m_deviceManager = nullptr;

    QString m_deviceId;
    QString m_stateTypeId;
    QString m_stateName;

    DeviceActionInfo *m_pendingActionInfo = nullptr;
    QVariant m_valueCache;

    QVariant m_valueStore;
};

}

#endif // SCRIPTSTATE_H
