#ifndef PLATFORM_H
#define PLATFORM_H

#include <QObject>

class PlatformPlugin;
class PlatformSystemController;
class PlatformUpdateController;

namespace nymeaserver {

class Platform : public QObject
{
    Q_OBJECT
public:
    explicit Platform(QObject *parent = nullptr);

    PlatformSystemController *systemController() const;
    PlatformUpdateController *updateController() const;

private:
    QStringList pluginSearchDirs() const;

private:
    PlatformPlugin *m_platformPlugin = nullptr;
};

}

#endif // PLATFORM_H
