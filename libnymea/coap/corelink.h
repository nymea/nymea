/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef CORELINK_H
#define CORELINK_H

#include <QObject>
#include <QDebug>

#include "libnymea.h"
#include "coappdu.h"

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
