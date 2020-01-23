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

#ifndef DEVICEDESCRIPTION_H
#define DEVICEDESCRIPTION_H

#include "libnymea.h"
#include "typeutils.h"
#include "types/param.h"

#include <QVariantMap>

class LIBNYMEA_EXPORT DeviceDescriptor
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QUuid deviceId READ deviceId USER true)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString description READ description)
    Q_PROPERTY(ParamList deviceParams READ params)

public:
    DeviceDescriptor();
    DeviceDescriptor(const DeviceClassId &deviceClassId, const QString &title = QString(), const QString &description = QString(), const DeviceId &parentDeviceId = DeviceId());
    DeviceDescriptor(const DeviceDescriptorId &id, const DeviceClassId &deviceClassId, const QString &title = QString(), const QString &description = QString(), const DeviceId &parentDeviceId = DeviceId());

    bool isValid() const;

    DeviceDescriptorId id() const;
    DeviceClassId deviceClassId() const;

    DeviceId deviceId() const;
    void setDeviceId(const DeviceId &deviceId);

    QString title() const;
    void setTitle(const QString &title);

    QString description() const;
    void setDescription(const QString &description);

    DeviceId parentDeviceId() const;
    void setParentDeviceId(const DeviceId &parentDeviceId);

    ParamList params() const;
    void setParams(const ParamList &params);

private:
    DeviceDescriptorId m_id;
    DeviceClassId m_deviceClassId;
    DeviceId m_deviceId;
    QString m_title;
    QString m_description;
    DeviceId m_parentDeviceId;
    ParamList m_params;
};

class DeviceDescriptors: public QList<DeviceDescriptor>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    DeviceDescriptors() {}
    inline DeviceDescriptors(std::initializer_list<DeviceDescriptor> args): QList(args) {}
    DeviceDescriptors(const QList<DeviceDescriptor> &other): QList<DeviceDescriptor>(other) {}
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};

Q_DECLARE_METATYPE(DeviceDescriptor)
Q_DECLARE_METATYPE(DeviceDescriptors)

#endif // DEVICEDESCRIPTION_H
