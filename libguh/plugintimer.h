#ifndef PLUGINTIMER_H
#define PLUGINTIMER_H

#include <QObject>
#include <QTimer>

#include "hardwareresource.h"

class PluginTimer : public HardwareResource
{
    Q_OBJECT
public:
    explicit PluginTimer(int intervall, QObject *parent = nullptr);

private:
    QTimer *m_timer = nullptr;
    int m_intervall = 10000;

signals:
    void timerEvent();

public slots:
    bool enable();
    bool disable();

};

#endif // PLUGINTIMER_H
