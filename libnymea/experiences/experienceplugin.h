#ifndef EXPERIENCEPLUGIN_H
#define EXPERIENCEPLUGIN_H

#include <QObject>

class DeviceManager;
class JsonRPCServer;

namespace nymeaserver {
class ExperienceManager;
}
class ExperiencePlugin : public QObject
{
    Q_OBJECT
public:
    explicit ExperiencePlugin(QObject *parent = nullptr);

    virtual void init() = 0;

protected:
    DeviceManager* deviceManager();
    JsonRPCServer* jsonRpcServer();

private:
    friend class nymeaserver::ExperienceManager;
    void initPlugin(DeviceManager *deviceManager, JsonRPCServer *jsonRPCServer);

    DeviceManager *m_deviceManager = nullptr;
    JsonRPCServer *m_jsonRpcServer = nullptr;

};

Q_DECLARE_INTERFACE(ExperiencePlugin, "io.nymea.ExperiencePlugin")


#endif // EXPERIENCEPLUGIN_H
