# Add more folders to ship with the application, here
folder_01.source = qml/hive_client
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

QT += core qml quick declarative


# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =
# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    hiveclientcore.cpp \
    settings.cpp \
    sensormodel.cpp

HEADERS += \
    hiveclientcore.h \
    settings.h \
    sensormodel.h


# Installation path
# target.path =

# Please do not modify the following two lines. Required for deployment.
include(qtquick2applicationviewer/qtquick2applicationviewer.pri)
qtcAddDeployment()



win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../libhive/release/ -llibhive
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../libhive/debug/ -llibhive
else:unix: LIBS += -L$$OUT_PWD/../../libhive/ -llibhive

INCLUDEPATH += $$PWD/../../libhive
DEPENDPATH += $$PWD/../../libhive

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../libhive/release/libhive.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../libhive/debug/libhive.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../libhive/liblibhive.a
