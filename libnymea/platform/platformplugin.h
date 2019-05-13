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

#ifndef PLATFORMPLUGIN_H
#define PLATFORMPLUGIN_H

#include <QObject>

#include "libnymea.h"

class PlatformSystemController;
class PlatformUpdateController;

class LIBNYMEA_EXPORT PlatformPlugin: public QObject
{
    Q_OBJECT
public:
    explicit PlatformPlugin(QObject *parent = nullptr);
    virtual ~PlatformPlugin() = default;

    virtual PlatformSystemController *systemController() const;
    virtual PlatformUpdateController *updateController() const;

private:
    PlatformSystemController *m_systemStub = nullptr;
    PlatformUpdateController *m_updateStub = nullptr;
};

Q_DECLARE_INTERFACE(PlatformPlugin, "io.nymea.PlatformPlugin")

#endif // PLATFORMPLUGIN_H
