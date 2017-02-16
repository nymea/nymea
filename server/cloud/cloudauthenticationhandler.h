/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef CLOUDAUTHENTICATIONHANDLER_H
#define CLOUDAUTHENTICATIONHANDLER_H

#include <QObject>

#include "cloudjsonhandler.h"

namespace guhserver {

class CloudAuthenticationHandler : public CloudJsonHandler
{
    Q_OBJECT
public:
    explicit CloudAuthenticationHandler(QObject *parent = 0);

    QString nameSpace() const;

public:
    enum CloudError {
        CloudErrorNoError,
        CloudErrorAuthenticationFailed,
        CloudErrorCloudConnectionDisabled,
        CloudErrorIdentityServerNotReachable,
        CloudErrorProxyServerNotReachable,
        CloudErrorLoginCredentialsMissing
    };

};

}

#endif // CLOUDAUTHENTICATIONHANDLER_H
