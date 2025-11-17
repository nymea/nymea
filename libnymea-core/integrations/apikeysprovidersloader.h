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

#ifndef APIKEYSPROVIDERSLOADER_H
#define APIKEYSPROVIDERSLOADER_H

#include "network/apikeys/apikeysprovider.h"

class ApiKeysProvidersLoader: public QObject
{
    Q_OBJECT

public:
    ApiKeysProvidersLoader(QObject *parent = nullptr);

    QList<ApiKeysProvider*> providers() const;

    QHash<QString, ApiKey> allApiKeys() const;

private:
    QStringList pluginSearchDirs() const;
    void loadPlugin(const QString &file);

    QList<ApiKeysProvider*> m_providers;
};

#endif // APIKEYSPROVIDERSLOADER_H
