#ifndef PLATFORMSYSTEMCONTROLLER_H
#define PLATFORMSYSTEMCONTROLLER_H

#include <QObject>

class PlatformSystemController : public QObject
{
    Q_OBJECT
public:
    explicit PlatformSystemController(QObject *parent = nullptr);
    virtual ~PlatformSystemController() = default;

    virtual bool powerManagementAvailable() const;
    virtual bool reboot();
    virtual bool shutdown();
};

#endif // PLATFORMSYSTEMCONTROLLER_H
