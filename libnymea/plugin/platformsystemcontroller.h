#ifndef PLATFORMSYSTEMCONTROLLER_H
#define PLATFORMSYSTEMCONTROLLER_H

#include <QObject>

class PlatformSystemController : public QObject
{
    Q_OBJECT
public:
    enum Capability {
        CapabilityNone = 0x00,
        CapabilityPower = 0x01,
        CapabilityAll = 0xFF
    };
    Q_ENUM(Capability)
    Q_DECLARE_FLAGS(Capabilities, Capability)

    explicit PlatformSystemController(QObject *parent = nullptr);
    virtual ~PlatformSystemController() = default;

    virtual Capabilities capabilities() const = 0;
    virtual bool reboot() = 0;
    virtual bool shutdown() = 0;
};

#endif // PLATFORMSYSTEMCONTROLLER_H
