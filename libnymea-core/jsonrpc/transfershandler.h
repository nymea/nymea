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

#ifndef TRANSFERSHANDLER_H
#define TRANSFERSHANDLER_H

#include "jsonrpc/jsonhandler.h"
#include "transfermanager.h"

namespace nymeaserver {

class TransfersHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit TransfersHandler(TransferManager *transferManager, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *CreateUpload(const QVariantMap &params, const JsonContext &context);
    Q_INVOKABLE JsonReply *StartDownload(const QVariantMap &params, const JsonContext &context);

signals:
    void DownloadAvailable(const QUuid &clientId, const QVariantMap &params);

private:
    TransferManager *m_transferManager = nullptr;
};

}

#endif // TRANSFERSHANDLER_H
