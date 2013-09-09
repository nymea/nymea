#ifndef RADIOPLUGIN_H
#define RADIOPLUGIN_H

#include <QObject>

class RadioPlugin : public QObject
{
    Q_OBJECT

public:
    explicit RadioPlugin(QObject *parent = 0);
    virtual ~RadioPlugin(){}

    virtual bool isValid(QList<int> rawData) = 0;
    virtual QByteArray getBinCode() = 0;

};
#endif // RADIOPLUGIN_H
