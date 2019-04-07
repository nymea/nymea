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

    virtual PlatformSystemController *systemController() const = 0;
    virtual PlatformUpdateController *updateController() const = 0;
};

Q_DECLARE_INTERFACE(PlatformPlugin, "io.nymea.PlatformPlugin")

#endif // PLATFORMPLUGIN_H
