/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#ifndef WEMOSWITCH_H
#define WEMOSWITCH_H

#include <QObject>

#include "plugin/deviceplugin.h"
#include "network/upnpdiscovery/upnpdevice.h"

class WemoSwitch : public UpnpDevice
{
    Q_OBJECT
public:
    explicit WemoSwitch(QObject *parent = 0, UpnpDeviceDescriptor upnpDeviceDescriptor = UpnpDeviceDescriptor());

    void setPowerState(const bool &powerState);
    bool powerState() const;

    void setReachable(const bool &reachable);
    bool reachable() const;

private:
    bool m_powerState;
    bool m_reachable;

};

#endif // WEMOSWITCH_H
