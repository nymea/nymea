#ifndef SCRIPTDEVICEPLUGIN_H
#define SCRIPTDEVICEPLUGIN_H

#include "devices/deviceplugin.h"

#include <QJSEngine>
#include <QJsonObject>

class ScriptDevicePlugin : public DevicePlugin
{
    Q_OBJECT
public:
    explicit ScriptDevicePlugin(QObject *parent = nullptr);

    bool loadScript(const QString &fileName);
    QJsonObject metaData() const;

    void init() override;
signals:

public slots:

private:
    QJSEngine *m_engine = nullptr;
    QJsonObject m_metaData;
};

#endif // SCRIPTDEVICEPLUGIN_H
