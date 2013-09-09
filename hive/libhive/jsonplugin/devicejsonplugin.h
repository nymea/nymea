#ifndef DEVICEJSONPLUGIN_H
#define DEVICEJSONPLUGIN_H

#include <QObject>
#include <QVariant>
#include <jsonplugin/jsonplugin.h>
#include <devicemanager.h>

class DeviceJsonPlugin : public JsonPlugin
{
    Q_OBJECT
public:
    explicit DeviceJsonPlugin(QObject *parent = 0);
    QString deviceName();
    QByteArray process(const QVariantMap &command, const QVariantMap &parameters);

private:
    DeviceManager *m_deviceManager;

    bool add();
    bool remove();
    bool edit();
    bool execute();

    QVariantMap m_command;
    QVariantMap m_parameters;


    QByteArray formatResponse();
    QByteArray formatErrorResponse();

signals:
    
    
};

#endif // DEVICEJSONPLUGIN_H
