// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef NETWORKHANDLER_H
#define NETWORKHANDLER_H

#include "jsonrpc/jsonhandler.h"

#include <QTimer>
#include <QVariantList>

namespace nymeaserver {

class NetworkHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit NetworkHandler(QObject *parent = nullptr);
    QString name() const override;

    Q_INVOKABLE JsonReply *GetBackendInfo(const QVariantMap &params);
    Q_INVOKABLE JsonReply *GetInterfaces(const QVariantMap &params);

signals:
    void BackendInfoChanged(const QVariantMap &params);
    void InterfacesChanged(const QVariantMap &params);

private slots:
    void refresh();

private:
    QVariantMap backendInfo() const;
    QVariantList interfaces() const;

    QTimer m_refreshTimer;
    QVariantMap m_backendInfo;
    QVariantList m_interfaces;
    QString m_selectedBackend;
    QString m_backendChoice;
};

}

#endif
