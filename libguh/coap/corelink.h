/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2016 Simon Stuerz <simon.stuerz@guh.guru>           *
 *                                                                         *
 *  This file is part of QtCoap.                                           *
 *                                                                         *
 *  QtCoap is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 3 of the License.                *
 *                                                                         *
 *  QtCoap is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with QtCoap. If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef CORELINK_H
#define CORELINK_H

#include <QObject>
#include <QDebug>

#include "coappdu.h"

class CoreLink
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
