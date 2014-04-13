/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "jsonhandler.h"

class DeviceHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit DeviceHandler(QObject *parent = 0);

    QString name() const override;

    Q_INVOKABLE QVariantMap GetSupportedVendors(const QVariantMap &params) const;

    Q_INVOKABLE QVariantMap GetSupportedDevices(const QVariantMap &params) const;

    Q_INVOKABLE QVariantMap GetPlugins(const QVariantMap &params) const;

    Q_INVOKABLE QVariantMap SetPluginConfiguration(const QVariantMap &params);

    Q_INVOKABLE QVariantMap AddConfiguredDevice(const QVariantMap &params);

    Q_INVOKABLE QVariantMap GetConfiguredDevices(const QVariantMap &params) const;

    Q_INVOKABLE QVariantMap RemoveConfiguredDevice(const QVariantMap &params);

    Q_INVOKABLE QVariantMap GetEventTypes(const QVariantMap &params) const;

    Q_INVOKABLE QVariantMap GetActionTypes(const QVariantMap &params) const;

    Q_INVOKABLE QVariantMap GetStateTypes(const QVariantMap &params) const;

    Q_INVOKABLE QVariantMap GetStateValue(const QVariantMap &params) const;

signals:
    void StateChanged(const QVariantMap &params);

private slots:
    void deviceStateChanged(Device *device, const QUuid &stateTypeId, const QVariant &value);
};

#endif // DEVICEHANDLER_H
