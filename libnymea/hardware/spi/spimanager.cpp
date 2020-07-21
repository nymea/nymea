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

#include "spimanager.h"

/*! \fn QStringList SPIManager::availablePorts() const
        Lists the available SPI ports in the system.
 */

/*! \fn QList<SPIScanResult> SPIManager::scanRegisters(const QString &portName = QString())
        Scans the SPI systems for connected devices. If \a portName is given, only this port
        will be scanned. If \a portName is empty, all available ports in the system will be
        scanned.
 */

/*! \fn bool SPIManager::open(SPIDevice *SPIDevice)
        Open the given SPIDevice. After reimplementing an SPIDevice, this method can be used
        to open the device. Returns false if opening fails, for example because of missing
        permissions or if the given \aSPIDevice is not valid, for example because of a bad
        port name or slave address.
*/

/*! \fn bool SPIManager::startReading(SPIDevice *spiDevice, int interval = 1000)
        Start reading from the given \a SPIDevice. When calling this, the SPIManager will start
        polling the given \a SPIDevice. Optionally, the interface can be given.
        Note that the interval might not be met, for example if the device is busy by other
        readers.
        The given \a SPIDevice is required to be opened first.
 */

/*! \fn void SPIManager::stopReading(SPIDevice *SPIDevice)
        Stops reading from the given \a SPIDevice.
 */

/*! \fn bool SPIManager::writeData(SPIDevice *SPIDevice, const QByteArray &data)
        Write the given \a data to the given \a SPIDevice.
        The data will be put into a write buffer and will be written to the device
        in a different thread when the SPI device is available.
        The given \a SPIDevice is required to be opened first.
 */

/*! \fn void SPIManager::close(SPIDevice *spiDevice)
        Closes the giben \a spiDevice. If reading are still active for this device,
        stopReading() will be called implicitly.
 */

SPIManager::SPIManager(QObject *parent):
    QObject(parent)
{

}
