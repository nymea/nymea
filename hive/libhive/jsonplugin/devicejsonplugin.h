#ifndef DEVICEJSONPLUGIN_H
#define DEVICEJSONPLUGIN_H

#include <QObject>
#include <QVariant>
#include <jsonplugin/jsonplugin.h>
#include <devicemanager.h>

class DeviceJsonPlugin : public JsonPlugin
{
public:
    explicit DeviceJsonPlugin(QObject *parent = 0);
    QString deviceName();
    QByteArray process(const QVariantMap &command, const QVariantMap &parameters);

private:
    DeviceManager *m_deviceManager;

    void add(QVariantMap parameters);
    void remove();
    void editValue(QString value, QVariant key);
    void getAll();

    QByteArray formatResponse();
    QByteArray formatErrorResponse();

signals:
    
    
};

#endif // DEVICEJSONPLUGIN_H
