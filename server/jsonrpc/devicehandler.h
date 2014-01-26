#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "jsonhandler.h"

class DeviceHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit DeviceHandler(QObject *parent = 0);

    QString name() const override;

    Q_INVOKABLE QVariantMap GetSupportedDevices(const QVariantMap &params) const;

    Q_INVOKABLE QVariantMap GetPlugins(const QVariantMap &params) const;

    Q_INVOKABLE QVariantMap SetPluginParams(const QVariantMap &params);

    Q_INVOKABLE QVariantMap AddConfiguredDevice(const QVariantMap &params);

    Q_INVOKABLE QVariantMap GetConfiguredDevices(const QVariantMap &params) const;

    Q_INVOKABLE QVariantMap GetEventTypes(const QVariantMap &params) const;

    Q_INVOKABLE QVariantMap GetActionTypes(const QVariantMap &params) const;

};

#endif // DEVICEHANDLER_H
