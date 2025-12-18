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

#ifndef PLATFORMZEROCONFCONTROLLER_H
#define PLATFORMZEROCONFCONTROLLER_H

#include "hardwareresource.h"
#include <QObject>

class ZeroConfServiceBrowser;
class ZeroConfServicePublisher;

class PlatformZeroConfController : public HardwareResource
{
    Q_OBJECT
public:
    explicit PlatformZeroConfController(QObject *parent = nullptr);
    virtual ~PlatformZeroConfController() = default;

    virtual ZeroConfServiceBrowser *createServiceBrowser(const QString &serviceType = QString());
    virtual ZeroConfServicePublisher *servicePublisher() const;

    // HardwareResource
    virtual bool available() const override;
    virtual bool enabled() const override;
    virtual void setEnabled(bool enabled) override;

private:
    ZeroConfServicePublisher *m_zeroConfPublisherDummy = nullptr;
};

Q_DECLARE_INTERFACE(PlatformZeroConfController, "io.nymea.PlatformZeroConfController")

#endif // PLATFORMZEROCONFCONTROLLER_H
