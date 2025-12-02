// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
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

#ifndef DEBUGHANDLER_H
#define DEBUGHANDLER_H

#include <QObject>

#include "jsonrpc/jsonhandler.h"

namespace nymeaserver {

class DebugHandler : public JsonHandler
{
    Q_OBJECT
public:
    enum DebugError {
        DebugErrorNoError
    };
    Q_ENUM(DebugError)

    enum LoggingCategoryType {
        LoggingCategoryTypeSystem,
        LoggingCategoryTypePlugin,
        LoggingCategoryTypeCustom
    };
    Q_ENUM(LoggingCategoryType)

    enum LoggingLevel {
        LoggingLevelCritical,
        LoggingLevelWarning,
        LoggingLevelInfo,
        LoggingLevelDebug
    };
    Q_ENUM(LoggingLevel)

    explicit DebugHandler(QObject *parent = nullptr);

    QString name() const override;

public slots:
    JsonReply* GetLoggingCategories(const QVariantMap &params);
    JsonReply* SetLoggingCategoryLevel(const QVariantMap &params);

signals:
    void LoggingCategoryLevelChanged(const QVariantMap &params);

};

}

#endif // DEBUGHANDLER_H
