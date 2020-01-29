/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "testhelper.h"

TestHelper* TestHelper::s_instance = nullptr;

TestHelper *TestHelper::instance()
{
    if (!s_instance) {
        s_instance = new TestHelper();
    }
    return s_instance;
}

void TestHelper::logEvent(const QString &deviceId, const QString &eventId, const QVariantMap &params)
{
    emit eventLogged(DeviceId(deviceId), eventId, params);
}

void TestHelper::logStateChange(const QString &deviceId, const QString &stateId, const QVariant &value)
{
    emit stateChangeLogged(DeviceId(deviceId), stateId, value);
}

TestHelper::TestHelper(QObject *parent) : QObject(parent)
{

}
