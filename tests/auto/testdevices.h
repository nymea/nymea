#ifndef TESTDEVICES_H
#define TESTDEVICES_H

#include "guhtestbase.h"

class TestDevices : public GuhTestBase
{
    Q_OBJECT
public:
    explicit TestDevices(QObject *parent = 0);
    TestDevices (const TestDevices &other) {}
    TestDevices& operator=(const TestDevices &other) {}

private slots:

    void getSupportedVendors();

    void getSupportedDevices_data();
    void getSupportedDevices();

    void addConfiguredDevice_data();
    void addConfiguredDevice();

    void getConfiguredDevices();

    void removeDevice();

    void storedDevices();

};

#endif // TESTDEVICES_H
