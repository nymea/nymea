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

#include "cloudauthenticationhandler.h"
#include "loggingcategories.h"
#include "guhcore.h"

#include <QUuid>


namespace guhserver {

CloudAuthenticationHandler::CloudAuthenticationHandler(QObject *parent) :
    CloudJsonHandler(parent)
{

}

QString CloudAuthenticationHandler::nameSpace() const
{
    return "Authentication";
}

void CloudAuthenticationHandler::processAuthenticate(const QVariantMap &params)
{
    if (params.contains("authenticationError")) {
        if (params.value("authenticationError").toString() == "AuthenticationErrorNoError") {
            if (params.contains("connectionId")) {
                GuhCore::instance()->cloudManager()->onConnectionAuthentificationFinished(true, params.value("connectionId").toUuid());
                return;
            }
        }
        qCWarning(dcCloud()) << "Authentication error:" << params.value("authenticationError").toString();
    }

    GuhCore::instance()->cloudManager()->onConnectionAuthentificationFinished(false, QUuid());
}

}
