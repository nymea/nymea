// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2026, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "transfershandler.h"

namespace nymeaserver {

TransfersHandler::TransfersHandler(TransferManager *transferManager, QObject *parent) :
    JsonHandler(parent),
    m_transferManager(transferManager)
{
    QString description;
    QVariantMap params;
    QVariantMap returns;

    description = "Create an upload session on the dedicated transfer connection.";
    params.insert("fileName", enumValueName(String));
    params.insert("size", enumValueName(Int));
    returns.insert("transferId", enumValueName(String));
    returns.insert("transferToken", enumValueName(String));
    returns.insert("fileName", enumValueName(String));
    returns.insert("size", enumValueName(Int));
    registerMethod("CreateUpload", description, params, returns, Types::PermissionScopeConfigureThings);

    description.clear();
    params.clear();
    returns.clear();
    description = "Create a download session for a previously announced download.";
    params.insert("downloadId", enumValueName(String));
    returns.insert("transferId", enumValueName(String));
    returns.insert("transferToken", enumValueName(String));
    returns.insert("fileName", enumValueName(String));
    returns.insert("size", enumValueName(Int));
    registerMethod("StartDownload", description, params, returns, Types::PermissionScopeControlThings);

    description.clear();
    params.clear();
    description = "Emitted when a completed upload can be downloaded on the dedicated transfer connection.";
    params.insert("downloadId", enumValueName(String));
    params.insert("fileName", enumValueName(String));
    params.insert("size", enumValueName(Int));
    registerNotification("DownloadAvailable", description, params);

    connect(m_transferManager, &TransferManager::downloadAvailable, this, &TransfersHandler::DownloadAvailable);
}

QString TransfersHandler::name() const
{
    return QStringLiteral("Transfers");
}

JsonReply *TransfersHandler::CreateUpload(const QVariantMap &params, const JsonContext &context)
{
    const auto info = m_transferManager->createUpload(params.value("fileName").toString(), params.value("size").toLongLong(), context);

    QVariantMap ret;
    ret.insert("transferId", info.transferId);
    ret.insert("transferToken", info.transferToken);
    ret.insert("fileName", info.fileName);
    ret.insert("size", info.size);
    return createReply(ret);
}

JsonReply *TransfersHandler::StartDownload(const QVariantMap &params, const JsonContext &context)
{
    const auto info = m_transferManager->createDownloadTransfer(params.value("downloadId").toString(), context);
    if (info.transferId.isEmpty() || info.transferToken.isEmpty()) {
        return createErrorReply(QStringLiteral("Unknown download"));
    }

    QVariantMap ret;
    ret.insert("transferId", info.transferId);
    ret.insert("transferToken", info.transferToken);
    ret.insert("fileName", info.fileName);
    ret.insert("size", info.size);
    return createReply(ret);
}

}
