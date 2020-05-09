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

#include "thingmanager.h"

/*!
    \class ThingManager
    \brief nymea's thing manager interface.

    \ingroup things
    \inmodule libnymea

    The ThingManager is the main entity managing all things in nymea.

    It is also responsible for loading Plugins and managing common hardware resources between
    \l{IntegrationPlugin}{integration plugins}.
*/

ThingManager::ThingManager(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<Param>();
    qRegisterMetaType<ParamList>();
    qRegisterMetaType<ParamType>();
    qRegisterMetaType<ParamTypes>();
}

IOConnectionResult ThingManager::connectIO(const ThingId &inputThing, const StateTypeId &inputState, const ThingId &outputThing, const StateTypeId &outputState, bool inverted)
{
    IOConnection connection(IOConnectionId::createIOConnectionId(), inputThing, inputState, outputThing, outputState, inverted);
    return connectIO(connection);
}
