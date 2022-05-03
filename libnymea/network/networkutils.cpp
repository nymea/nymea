#include "networkutils.h"

NetworkUtils::NetworkUtils()
{

}

QNetworkInterface NetworkUtils::getInterfaceForHostaddress(const QHostAddress &address)
{
    foreach (const QNetworkInterface &networkInterface, QNetworkInterface::allInterfaces()) {
        foreach (const QNetworkAddressEntry &entry, networkInterface.addressEntries()) {
            // Only IPv4
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            if (address.isInSubnet(entry.ip(), entry.prefixLength())) {
                return networkInterface;
            }
        }
    }

    return QNetworkInterface();
}

QNetworkInterface NetworkUtils::getInterfaceForMacAddress(const QString &macAddress)
{
    foreach (const QNetworkInterface &networkInterface, QNetworkInterface::allInterfaces()) {
        if (networkInterface.hardwareAddress().toLower() == macAddress.toLower()) {
            return networkInterface;
        }
    }

    return QNetworkInterface();
}

QNetworkInterface NetworkUtils::getInterfaceForMacAddress(const MacAddress &macAddress)
{
    foreach (const QNetworkInterface &networkInterface, QNetworkInterface::allInterfaces()) {
        if (MacAddress(networkInterface.hardwareAddress()) == macAddress) {
            return networkInterface;
        }
    }

    return QNetworkInterface();
}
