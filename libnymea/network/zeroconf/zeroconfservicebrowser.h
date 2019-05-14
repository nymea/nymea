/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016-2018 Simon Stürz <simon.stuerz@guh.io>              *
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

#ifndef ZEROCONFSERVICEBROWSER_H
#define ZEROCONFSERVICEBROWSER_H

#include <QObject>

#include "libnymea.h"
#include "hardwareresource.h"
#include "zeroconfserviceentry.h"

class LIBNYMEA_EXPORT ZeroConfServiceBrowser : public HardwareResource
{
    Q_OBJECT

public:
    explicit ZeroConfServiceBrowser(QObject *parent = nullptr);
    virtual ~ZeroConfServiceBrowser() = default;

    virtual bool available() const override;
    virtual bool enabled() const override;
    virtual void setEnabled(bool enabled) override;


    virtual QList<ZeroConfServiceEntry> serviceEntries() const;

signals:
    void serviceEntryAdded(const ZeroConfServiceEntry &entry);
    void serviceEntryRemoved(const ZeroConfServiceEntry &entry);

};

#endif // ZEROCONFSERVICEBROWSER_H
