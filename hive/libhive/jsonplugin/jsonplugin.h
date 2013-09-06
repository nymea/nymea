#ifndef JSONPLUGIN_H
#define JSONPLUGIN_H

#include <QObject>

class JsonPlugin : public QObject
{
    Q_OBJECT
public:
    explicit JsonPlugin(QObject *parent = 0);
    virtual ~JsonPlugin(){}
    virtual QString deviceName() = 0;
    virtual QByteArray process(const QVariantMap & command, const QVariantMap & parameters) = 0;


signals:
    
public slots:
    
};

#endif // JSONPLUGIN_H
