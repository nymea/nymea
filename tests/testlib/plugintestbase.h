#ifndef PLUGINTESTBASE_H
#define PLUGINTESTBASE_H

#include <QObject>

#include "integrations/thingmanagerimplementation.h"
#include "hardwaremanagermock.h"

class TestPlugins: public QObject
{
    Q_OBJECT
public:
    TestPlugins(const QString &pluginPath);
private slots:

    void init();

    void testTest();

private:
    HardwareManager *m_hardwareManager = nullptr;
    ThingManager *m_thingManager = nullptr;

};

#define NYMEA_PLUGIN_TEST_MAIN(TestObject) \
QT_BEGIN_NAMESPACE \
QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS \
QT_END_NAMESPACE \
int main(int argc, char *argv[]) \
{ \
    QGuiApplication app(argc, argv); \
    app.setAttribute(Qt::AA_Use96Dpi, true); \
    QTEST_ADD_GPU_BLACKLIST_SUPPORT \
    TestObject tc; \
    QTEST_SET_MAIN_SOURCE_PATH \
    return QTest::qExec(&tc, argc, argv); \
}

#endif
