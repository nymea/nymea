#ifndef HIVECORE_H
#define HIVECORE_H

#include "rule.h"
#include "event.h"

#include <QObject>

class JsonRPCServer;
class DeviceManager;
class RuleEngine;

class GuhCore : public QObject
{
    Q_OBJECT
public:
    static GuhCore* instance();

    DeviceManager* deviceManager() const;
    RuleEngine *ruleEngine() const;

private:
    explicit GuhCore(QObject *parent = 0);
    static GuhCore *s_instance;

    JsonRPCServer *m_jsonServer;
    DeviceManager *m_deviceManager;
    RuleEngine *m_ruleEngine;

private slots:
    void gotSignal(const Event &event);

};

#endif // HIVECORE_H
