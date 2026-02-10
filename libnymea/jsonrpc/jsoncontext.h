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

#ifndef JSONCONTEXT_H
#define JSONCONTEXT_H

#include <QUuid>
#include <QLocale>

class JsonContext
{
public:
    JsonContext(const QUuid &clientId, const QLocale &locale, bool authenticationEnabled = true);

    QUuid clientId() const;
    QLocale locale() const;

    QByteArray token() const;
    void setToken(const QByteArray &token);

    bool authenticationEnabled() const;
    void setAuthenticationEnabled(bool authenticationEnabled);

private:
    QUuid m_clientId;
    QLocale m_locale;
    QByteArray m_token;
    bool m_authenticationEnabled = true;
};

#endif // JSONCONTEXT_H
