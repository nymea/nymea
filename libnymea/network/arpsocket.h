#ifndef ARPSOCKET_H
#define ARPSOCKET_H

#include <QDebug>
#include <QObject>
#include <QSocketNotifier>
#include <QLoggingCategory>
#include <QHostAddress>
#include <QNetworkInterface>

class ArpSocket : public QObject
{
    Q_OBJECT
public:
    explicit ArpSocket(QObject *parent = nullptr);

    // Send ARP request to all local networks
    bool sendRequest();

    // Send ARP request to a specific network interface with the given name
    bool sendRequest(const QString &interfaceName);

    // Send ARP request to a specific network interface
    bool sendRequest(const QNetworkInterface &networkInterface);

    // Send ARP request to a specific address within the network
    bool sendRequest(const QHostAddress &targetAddress);

    bool isOpen() const;

    bool openSocket();
    void closeSocket();

signals:
    void arpResponse(const QNetworkInterface &networkInterface, const QHostAddress &address, const QString &macAddress);

private:
    QSocketNotifier *m_socketNotifier = nullptr;
    int m_socketDescriptor = -1;
    bool m_isOpen = false;

    bool sendRequestInternally(int networkInterfaceIndex, const QString &senderMacAddress, const QHostAddress &senderHostAddress, const QString &targetMacAddress, const QHostAddress &targetHostAddress);

    QString getMacAddressString(uint8_t *senderHardwareAddress);
    QHostAddress getHostAddressString(uint8_t *senderIpAddress);

    void fillMacAddress(uint8_t *targetArray, const QString &macAddress);
    void fillHostAddress(uint8_t *targetArray, const QHostAddress &hostAddress);

    QNetworkInterface getInterfaceForHostaddress(const QHostAddress &address);
    QNetworkInterface getInterfaceForMacAddress(const QString &macAddress);
};

#endif // ARPSOCKET_H
