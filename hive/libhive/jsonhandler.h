#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include <QObject>
#include <QVariant>
#include <jsonplugin/jsonplugin.h>
#include <jsonplugin/devicejsonplugin.h>
#include <jsonplugin/radiojsonplugin.h>


class JsonHandler : public QObject
{
    Q_OBJECT
public:
    explicit JsonHandler(QObject *parent = 0);
    QByteArray addRemote(QString deviceType, QString method, QString name, QString protocol, QString Linecode, QString m);


signals:
    void notifyAll(const QByteArray &data);

    
public slots:
    QByteArray process(const QByteArray &data);

private:
    DeviceJsonPlugin *m_device;
    RadioJsonPlugin *m_radio;

    QByteArray formatResponse(const QVariantMap &command, const QVariantMap &responseParams);
    QByteArray formatErrorResponse(const QVariantMap &command, const QString &error);

    
};

#endif // JSONHANDLER_H
