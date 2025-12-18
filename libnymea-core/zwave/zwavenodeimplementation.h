// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef ZWAVENODEIMPLEMENTATION_H
#define ZWAVENODEIMPLEMENTATION_H

#include "hardware/zwave/zwavenode.h"

namespace nymeaserver {
class ZWaveManager;

class ZWaveNodeImplementation : public ZWaveNode
{
    Q_OBJECT

public:
    explicit ZWaveNodeImplementation(ZWaveManager *manager, const QUuid &networkUuid, quint8 nodeId, QObject *parent = nullptr);

    QUuid networkUuid() const override;

    quint8 nodeId() const override;
    ZWaveNodeType nodeType() const override;
    void setNodeType(ZWaveNodeType nodeType);

    ZWaveNodeRole role() const override;
    void setRole(ZWaveNodeRole role);

    bool reachable() const override;
    void setReachable(bool reachable);

    bool initialized() const override;
    void setInitialized(bool initialized);

    bool failed() const override;
    void setFailed(bool failed);

    bool sleeping() const override;
    void setSleeping(bool sleeping);

    quint8 linkQuality() const override;
    void setLinkQuality(quint8 linkQuality);

    quint8 securityMode() const override;
    void setSecurityMode(quint8 securityMode);

    ZWaveDeviceType deviceType() const override;
    void setDeviceType(ZWaveDeviceType deviceType);

    ZWavePlusDeviceType plusDeviceType() const override;
    void setPlusDeviceType(ZWavePlusDeviceType plusDeviceType);

    quint16 manufacturerId() const override;
    void setManufacturerId(quint16 manufacturerId);

    QString manufacturerName() const override;
    void setManufacturerName(const QString &manufacturerName);

    QString name() const override;
    void setName(const QString &name);

    quint16 productId() const override;
    void setProductId(quint16 productId);

    QString productName() const override;
    void setProductName(const QString &productName);

    quint16 productType() const override;
    void setProductType(quint16 productType);

    quint8 version() const override;
    void setVersion(quint8 version);

    bool isZWavePlusDevice() const override;
    void setIsZWavePlusDevice(bool isZWavePlusDevice);

    bool isSecurityDevice() const override;
    void setIsSecurityDevice(bool isSecurityDevice);

    bool isBeamingDevice() const override;
    void setIsBeamingDevice(bool isBeamingDevice);

    QList<ZWaveValue> values() const override;

    ZWaveValue value(quint64 valueId) const override;
    ZWaveValue value(ZWaveValue::Genre genre, ZWaveValue::CommandClass commandClass, quint8 instance, quint16 index, ZWaveValue::Type type) const override;
    void updateValue(const ZWaveValue &value);
    void removeValue(quint64 id);

    void setValue(const ZWaveValue &value) override;

signals:
    // For convenience, emitted when anything in the node changes
    void nodeChanged();

private:
    nymeaserver::ZWaveManager *m_manager = nullptr;
    QUuid m_networkUuid;
    quint8 m_nodeId;

    bool m_initialized = false;
    bool m_reachable = false;
    bool m_failed = false;
    bool m_sleeping = false;
    quint8 m_linkQuality = 0;
    quint8 m_securityMode = 0;

    ZWaveNodeType m_nodeType = ZWaveNodeTypeUnknown;
    ZWaveNodeRole m_role = ZWaveNodeRoleUnknown;
    ZWaveDeviceType m_deviceType = ZWaveDeviceTypeUnknown;
    ZWavePlusDeviceType m_plusDeviceType = ZWavePlusDeviceTypeUnknown;

    quint16 m_manufacturerId;
    QString m_manufacturerName;

    QString m_name;

    quint16 m_productId;
    QString m_productName;
    quint16 m_productType;

    quint8 m_version;
    bool m_isZWavePlusDevice = false;
    bool m_isSecurityDevice = false;
    bool m_isBeamingDevice = false;

    QHash<quint64, ZWaveValue> m_values;
};

} // namespace nymeaserver

#endif // ZWAVENODEIMPLEMENTATION_H
