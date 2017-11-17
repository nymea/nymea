/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
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

#ifndef HARDWARERESOURCE_H
#define HARDWARERESOURCE_H

#include <QObject>

class HardwareResource : public QObject
{
    Q_OBJECT
public:
    enum Type {
        TypeNone = 0,
        TypeRadio433 = 1,
        TypeTimer = 2,
        TypeNetworkManager = 4,
        TypeUpnpDisovery = 8,
        TypeBluetoothLE = 16,
        TypeAvahiBrowser = 32
    };
    Q_ENUM(Type)
    Q_DECLARE_FLAGS(Types, Type)

    explicit HardwareResource(const HardwareResource::Type &hardwareReourceType, const QString &name, QObject *parent = nullptr);

    HardwareResource::Type hardwareReourceType() const;

    QString name() const;

    bool available() const;
    bool enabled() const;

private:
    HardwareResource::Type m_hardwareReourceType;
    QString m_name;
    // Note: default enabled, but not available. Each hardwareresource has explicitly chek if available
    bool m_available = false;
    bool m_enabled = true;

protected:
    void setEnabled(const bool &enabled);
    void setAvailable(const bool &available);

signals:
    void enabledChanged(const bool &enabled);
    void availableChanged(const bool &available);

public slots:
    virtual bool enable() = 0;
    virtual bool disable() = 0;

};

Q_DECLARE_METATYPE(HardwareResource::Type)
Q_DECLARE_OPERATORS_FOR_FLAGS(HardwareResource::Types)

#endif // HARDWARERESOURCE_H
