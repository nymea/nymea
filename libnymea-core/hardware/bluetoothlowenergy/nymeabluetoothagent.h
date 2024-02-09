#ifndef NYMEABLUETOOTHAGENT_H
#define NYMEABLUETOOTHAGENT_H

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusMessage>
#include <QBluetoothAddress>

#include "nymeabluetoothagent.h"

namespace nymeaserver
{

class NymeaBluetoothAgentAdapter: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent1")

public:
    explicit NymeaBluetoothAgentAdapter(NymeaBluetoothAgent *agent, QObject *parent = nullptr);

public slots:
    Q_SCRIPTABLE QString RequestPinCode(const QDBusObjectPath &device, const QDBusMessage &message);
    Q_SCRIPTABLE void DisplayPinCode(const QDBusObjectPath &device, const QString &pinCode);
    Q_SCRIPTABLE quint32 RequestPasskey(const QDBusObjectPath &device, const QDBusMessage &message);
    Q_SCRIPTABLE void DisplayPasskey(const QDBusObjectPath &device, quint32 passKey, quint16 entered);

    Q_SCRIPTABLE void RequestConfirmation(const QDBusObjectPath &device, quint32 passKey, const QDBusMessage &message);
    Q_SCRIPTABLE void RequestAuthorization(const QDBusObjectPath &device, const QDBusMessage &message);
    Q_SCRIPTABLE void AuthorizeService(const QDBusObjectPath &device, const QString &uuid, const QDBusMessage &message);

    Q_SCRIPTABLE void Cancel();
    Q_SCRIPTABLE void Release();

private:
    NymeaBluetoothAgent *m_agent = nullptr;

};

class NymeaBluetoothAgent : public QObject
{
    Q_OBJECT

public:
    explicit NymeaBluetoothAgent(QObject *parent = nullptr);

    void passKeyEntered(const QBluetoothAddress &address, const QString passKey);

signals:
    void passKeyRequested(const QBluetoothAddress &address);
    void displayPinCode(const QBluetoothAddress &address, const QString &pinCode);

private:
    friend class NymeaBluetoothAgentAdapter;
    QBluetoothAddress deviceForPath(const QDBusObjectPath &path);
    void onRequestPassKey(const QDBusObjectPath &path, const QDBusMessage &message);
    void onDisplayPinCode(const QDBusObjectPath &path, const QString &pinCode);
    NymeaBluetoothAgentAdapter *m_adapter = nullptr;
    QHash<QString, QDBusMessage> m_pendingPairings;
};

}

#endif // NYMEABLUETOOTHAGENT_H
