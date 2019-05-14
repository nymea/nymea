/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
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

#ifndef PLATFORMZEROCONFCONTROLLER_H
#define PLATFORMZEROCONFCONTROLLER_H

#include <QObject>

class ZeroConfServiceBrowser;
class ZeroConfServicePublisher;

class PlatformZeroConfController: public QObject
{
    Q_OBJECT
public:
    explicit PlatformZeroConfController(QObject *parent = nullptr);
    virtual ~PlatformZeroConfController() = default;

    virtual ZeroConfServiceBrowser *zeroConfServiceBrowser() const;
    virtual ZeroConfServicePublisher *zeroConfServicePublisher() const;

private:
    ZeroConfServiceBrowser *m_zeroConfBrowserStub = nullptr;
    ZeroConfServicePublisher *m_zeroConfPublisherStub = nullptr;
};

Q_DECLARE_INTERFACE(PlatformZeroConfController, "io.nymea.PlatformZeroConfController")

#endif // PLATFORMZEROCONFCONTROLLER_H
