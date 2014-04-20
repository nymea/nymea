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

#include <QCoreApplication>
#include <guhcore.h>

#include <QtPlugin>

Q_IMPORT_PLUGIN(DevicePluginElro)
Q_IMPORT_PLUGIN(DevicePluginIntertechno)
//Q_IMPORT_PLUGIN(DevicePluginMeisterAnker)
Q_IMPORT_PLUGIN(DevicePluginWifiDetector)
//Q_IMPORT_PLUGIN(DevicePluginConrad)
Q_IMPORT_PLUGIN(DevicePluginMock)
//Q_IMPORT_PLUGIN(DevicePluginWeatherground)
Q_IMPORT_PLUGIN(DevicePluginOpenweathermap)
Q_IMPORT_PLUGIN(DevicePluginLircd)
Q_IMPORT_PLUGIN(DevicePluginGoogleMail)

#if USE_BOBLIGHT
Q_IMPORT_PLUGIN(DevicePluginBoblight)
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setOrganizationName("guh");

    GuhCore::instance();

    return a.exec();
}
