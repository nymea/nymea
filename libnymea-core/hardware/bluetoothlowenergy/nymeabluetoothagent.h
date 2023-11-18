#ifndef NYMEABLUETOOTHAGENT_H
#define NYMEABLUETOOTHAGENT_H

#include <QObject>

class NymeaBluetoothAgent : public QObject
{
    Q_OBJECT
public:
    explicit NymeaBluetoothAgent(QObject *parent = nullptr);

signals:

};

#endif // NYMEABLUETOOTHAGENT_H
