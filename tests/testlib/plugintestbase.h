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
    QCommandLineParser p; \
    p.addHelpOption(); \
    p.addPositionalArgument("pluginPath", "The path to the plugin ib to be tested."); \
    p.process(app); \
    if (parser.positionalArguments().count() != 1) qWarning() << qUtf8Printable(parser.helpText()); \
    app.setAttribute(Qt::AA_Use96Dpi, true); \
    QTEST_ADD_GPU_BLACKLIST_SUPPORT \
    TestObject tc(p.positionalArguments().first()); \
    QTEST_SET_MAIN_SOURCE_PATH \
    return QTest::qExec(&tc, argc, argv); \
}

#endif
