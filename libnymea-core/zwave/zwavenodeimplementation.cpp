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

#include "zwavenodeimplementation.h"
#include <QUuid>
#include <QDebug>

#include "zwavemanager.h"

namespace nymeaserver
{

nymeaserver::ZWaveNodeImplementation::ZWaveNodeImplementation(ZWaveManager *manager, const QUuid &networkUuid, quint8 nodeId, QObject *parent):
    ZWaveNode{parent},
    m_manager(manager),
    m_networkUuid(networkUuid),
    m_nodeId(nodeId)
{

}

QUuid ZWaveNodeImplementation::networkUuid() const
{
    return m_networkUuid;
}

quint8 ZWaveNodeImplementation::nodeId() const
{
    return m_nodeId;
}

ZWaveNodeImplementation::ZWaveNodeType ZWaveNodeImplementation::nodeType() const
{
    return m_nodeType;
}

void ZWaveNodeImplementation::setNodeType(ZWaveNodeType nodeType)
{
    if (m_nodeType != nodeType) {
        m_nodeType = nodeType;
        emit nodeTypeChanged();
        emit nodeChanged();
    }
}

ZWaveNode::ZWaveNodeRole ZWaveNodeImplementation::role() const
{
    return m_role;
}

void ZWaveNodeImplementation::setRole(ZWaveNodeRole role)
{
    if (m_role != role) {
        m_role = role;
        emit roleChanged();
        emit nodeChanged();
    }
}

ZWaveNodeImplementation::ZWaveDeviceType ZWaveNodeImplementation::deviceType() const
{
    return m_deviceType;
}

void ZWaveNodeImplementation::setDeviceType(ZWaveDeviceType deviceType)
{
    if (m_deviceType != deviceType) {
        m_deviceType = deviceType;
        emit deviceTypeChanged();
        emit nodeChanged();
    }
}

ZWaveNode::ZWavePlusDeviceType ZWaveNodeImplementation::plusDeviceType() const
{
    return m_plusDeviceType;
}

void ZWaveNodeImplementation::setPlusDeviceType(ZWavePlusDeviceType plusDeviceType)
{
    if (m_plusDeviceType != plusDeviceType) {
        m_plusDeviceType = plusDeviceType;
        emit plusDeviceTypeChanged();
        emit nodeChanged();
    }
}

quint16 ZWaveNodeImplementation::manufacturerId() const
{
    return m_manufacturerId;
}

void ZWaveNodeImplementation::setManufacturerId(quint16 manufacturerId)
{
    if (m_manufacturerId != manufacturerId) {
        m_manufacturerId = manufacturerId;
        emit manufacturerIdChanged();
        emit nodeChanged();
    }
}

QString ZWaveNodeImplementation::manufacturerName() const
{
    return m_manufacturerName;
}

void ZWaveNodeImplementation::setManufacturerName(const QString &manufacturerName)
{
    if (m_manufacturerName != manufacturerName) {
        m_manufacturerName = manufacturerName;
        emit manufacturerNameChanged();
        emit nodeChanged();
    }
}

QString ZWaveNodeImplementation::name() const
{
    return m_name;
}

void ZWaveNodeImplementation::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
        emit nodeChanged();
    }
}

quint16 ZWaveNodeImplementation::productId() const
{
    return m_productId;
}

void ZWaveNodeImplementation::setProductId(quint16 productId)
{
    if (m_productId != productId) {
        m_productId = productId;
        emit productIdChanged();
        emit nodeChanged();
    }
}

QString ZWaveNodeImplementation::productName() const
{
    return m_productName;
}

void ZWaveNodeImplementation::setProductName(const QString &productName)
{
    if (m_productName != productName) {
        m_productName = productName;
        emit productNameChanged();
        emit nodeChanged();
    }
}

quint16 ZWaveNodeImplementation::productType() const
{
    return m_productType;
}

void ZWaveNodeImplementation::setProductType(quint16 productType)
{
    if (m_productType != productType) {
        m_productType = productType;
        emit productTypeChanged();
        emit nodeChanged();
    }
}

quint8 ZWaveNodeImplementation::version() const
{
    return m_version;
}

void ZWaveNodeImplementation::setVersion(quint8 version)
{
    if (m_version != version) {
        m_version = version;
        emit versionChanged();
        emit nodeChanged();
    }
}

bool ZWaveNodeImplementation::isZWavePlusDevice() const
{
    return m_isZWavePlusDevice;
}

void ZWaveNodeImplementation::setIsZWavePlusDevice(bool isZWavePlusDevice)
{
    if (m_isZWavePlusDevice != isZWavePlusDevice) {
        m_isZWavePlusDevice = isZWavePlusDevice;
        emit isZWavePlusDeviceChanged();
        emit nodeChanged();
    }
}

bool ZWaveNodeImplementation::isSecurityDevice() const
{
    return m_isSecurityDevice;
}

void ZWaveNodeImplementation::setIsSecurityDevice(bool isSecurityDevice)
{
    if (m_isSecurityDevice != isSecurityDevice) {
        m_isSecurityDevice = isSecurityDevice;
        emit isSecurityDeviceChanged();
        emit nodeChanged();
    }
}

bool ZWaveNodeImplementation::isBeamingDevice() const
{
    return m_isBeamingDevice;
}

void ZWaveNodeImplementation::setIsBeamingDevice(bool isBeamingDevice)
{
    if (m_isBeamingDevice != isBeamingDevice) {
        m_isBeamingDevice = isBeamingDevice;
        emit isBeamingDeviceChanged();
        emit nodeChanged();
    }
}

void ZWaveNodeImplementation::updateValue(const ZWaveValue &value)
{
    if (m_values.contains(value.id())) {
        m_values[value.id()] = value;
        emit valueChanged(value);
    } else {
        m_values.insert(value.id(), value);
        emit valueAdded(value);
    }
}

void ZWaveNodeImplementation::removeValue(quint64 id)
{
    if (m_values.contains(id)) {
        emit valueRemoved(m_values.take(id));
    }
}

QList<ZWaveValue> ZWaveNodeImplementation::values() const
{
    return m_values.values();
}

ZWaveValue ZWaveNodeImplementation::value(quint64 valueId) const
{
    return m_values.value(valueId);
}

ZWaveValue ZWaveNodeImplementation::value(ZWaveValue::Genre genre, ZWaveValue::CommandClass commandClass, quint8 instance, quint16 index, ZWaveValue::Type type) const
{
    foreach (const ZWaveValue &value, m_values) {
        if (value.genre() == genre && value.commandClass() == commandClass && value.instance() == instance && value.index() == index && value.type() == type) {
            return value;
        }
    }
    return ZWaveValue();
}

void ZWaveNodeImplementation::setValue(const ZWaveValue &value)
{
    m_manager->setValue(m_networkUuid, m_nodeId, value);
}

bool ZWaveNodeImplementation::reachable() const
{
    return m_reachable;
}

void ZWaveNodeImplementation::setReachable(bool reachable)
{
    if (m_reachable != reachable) {
        m_reachable = reachable;
        emit reachableChanged(reachable);
        emit nodeChanged();
    }
}

bool ZWaveNodeImplementation::initialized() const
{
    return m_initialized;
}

void ZWaveNodeImplementation::setInitialized(bool initialized)
{
    if (m_initialized != initialized) {
        m_initialized = initialized;
        emit initializedChanged(initialized);
        emit nodeChanged();
    }
}

bool ZWaveNodeImplementation::failed() const
{
    return m_failed;
}

void ZWaveNodeImplementation::setFailed(bool failed)
{
    if (m_failed != failed) {
        m_failed = failed;
        emit failedChanged(failed);
        emit nodeChanged();
    }
}

bool ZWaveNodeImplementation::sleeping() const
{
    return m_sleeping;
}

void ZWaveNodeImplementation::setSleeping(bool sleeping)
{
    if (m_sleeping != sleeping) {
        m_sleeping = sleeping;
        emit sleepingChanged(sleeping);
        emit nodeChanged();
    }
}

quint8 ZWaveNodeImplementation::linkQuality() const
{
    return m_linkQuality;
}

void ZWaveNodeImplementation::setLinkQuality(quint8 linkQuality)
{
    if (m_linkQuality != linkQuality) {
        m_linkQuality = linkQuality;
        emit linkQualityChanged(linkQuality);
        emit nodeChanged();
    }
}

quint8 ZWaveNodeImplementation::securityMode() const
{
    return m_securityMode;
}

void ZWaveNodeImplementation::setSecurityMode(quint8 securityMode)
{
    if (m_securityMode != securityMode) {
        m_securityMode = securityMode;
        emit securityModeChanged(securityMode);
        emit nodeChanged();
    }
}

}
