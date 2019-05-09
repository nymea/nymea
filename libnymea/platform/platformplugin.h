#ifndef PLATFORMPLUGIN_H
#define PLATFORMPLUGIN_H

#include <QObject>

#include "libnymea.h"

class PlatformSystemController;
class PlatformUpdateController;

class LIBNYMEA_EXPORT PlatformPlugin: public QObject
{
    Q_OBJECT
public:
    explicit PlatformPlugin(QObject *parent = nullptr);
    virtual ~PlatformPlugin() = default;

    virtual PlatformSystemController *systemController() const;
    virtual PlatformUpdateController *updateController() const;

private:
    PlatformSystemController *m_systemStub = nullptr;
    PlatformUpdateController *m_updateStub = nullptr;
};

Q_DECLARE_INTERFACE(PlatformPlugin, "io.nymea.PlatformPlugin")

#endif // PLATFORMPLUGIN_H
