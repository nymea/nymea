#ifndef CLOUDINTERFACE_H
#define CLOUDINTERFACE_H

#include <QObject>

class CloudInterface : public QObject
{
    Q_OBJECT
public:
    explicit CloudInterface(QObject *parent = 0);

signals:

public slots:
};

#endif // CLOUDINTERFACE_H
