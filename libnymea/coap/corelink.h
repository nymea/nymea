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

#ifndef CORELINK_H
#define CORELINK_H

#include <QDebug>
#include <QObject>

#include "coappdu.h"
#include "libnymea.h"

class LIBNYMEA_EXPORT CoreLink
{
public:
    CoreLink();

    QString path() const;
    void setPath(const QString &path);

    // link params
    QString title() const;
    void setTitle(const QString &title);

    QString resourceType() const;
    void setResourceType(const QString &resourceType);

    QString interfaceDescription() const;
    void setInterfaceDescription(const QString &interfaceDescription);

    CoapPdu::ContentType contentType() const;
    void setContentType(const CoapPdu::ContentType &contentType);

    int maximumSize() const;
    void setMaximumSize(const int &maximumSize);

    bool observable() const;
    void setObservable(const bool &observable);

private:
    QString m_path;

    QString m_title;
    QString m_resourceType;
    QString m_interfaceDescription;
    CoapPdu::ContentType m_contentType;
    int m_maximumSize;
    bool m_observable;
};

QDebug operator<<(QDebug debug, const CoreLink &link);

#endif // CORELINK_H
