#ifndef TESTVERSIONING_H
#define TESTVERSIONING_H

#include "guhtestbase.h"

class TestVersioning: public GuhTestBase
{
    Q_OBJECT
public:
    TestVersioning () {}
    TestVersioning (const TestVersioning &other) {}
    TestVersioning& operator=(const TestVersioning &other) {}

private slots:
    void version();
    void apiChangeBumpsVersion();
};
Q_DECLARE_METATYPE(TestVersioning);

#endif //TESTVERSIONING_H
