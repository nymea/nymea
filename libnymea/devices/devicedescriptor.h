/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEVICEDESCRIPTION_H
#define DEVICEDESCRIPTION_H

#include "libnymea.h"
#include "typeutils.h"
#include "types/param.h"

#include <QVariantMap>

class LIBNYMEA_EXPORT DeviceDescriptor
{
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

Q_DECLARE_METATYPE(DeviceDescriptor)

#endif // DEVICEDESCRIPTION_H
