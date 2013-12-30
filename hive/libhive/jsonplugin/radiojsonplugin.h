#ifndef RADIOJSONPLUGIN_H
#define RADIOJSONPLUGIN_H

#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <jsonplugin/jsonplugin.h>
#include <devicemanager.h>

class RadioJsonPlugin : public JsonPlugin
{
    Q_OBJECT
public:
    explicit RadioJsonPlugin(QObject *parent = 0);

    QString deviceName();
    QStringList devicePropertys();
    QStringList deviceMethods();

    QByteArray process(const QVariantMap & command, const QVariantMap & parameters);

private:
    // methods
    bool add();
    bool remove();
    bool edit();
    bool execute();

    QVariantMap m_command;
    QVariantMap m_parameters;
    DeviceManager *m_deviceManager;

    QByteArray formatResponse();
    QByteArray formatErrorResponse();

signals:

public slots:

};

#endif // RADIOJSONPLUGIN_H
