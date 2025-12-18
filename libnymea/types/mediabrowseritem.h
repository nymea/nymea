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

#ifndef MEDIABROWSERITEM_H
#define MEDIABROWSERITEM_H

#include "browseritem.h"

class MediaBrowserItem : public BrowserItem
{
    Q_GADGET
    Q_PROPERTY(MediaBrowserIcon mediaIcon READ mediaIcon)

public:
    enum MediaBrowserIcon {
        MediaBrowserIconNone = 1,
        MediaBrowserIconPlaylist = 2,
        MediaBrowserIconRecentlyPlayed = 3,
        MediaBrowserIconLibrary = 4,
        MediaBrowserIconMusicLibrary = 5,
        MediaBrowserIconVideoLibrary = 6,
        MediaBrowserIconPictureLibrary = 7,

        MediaBrowserIconDisk = 100,
        MediaBrowserIconUSB = 101,
        MediaBrowserIconNetwork = 102,
        MediaBrowserIconAux = 103,
        MediaBrowserIconBluetooth = 104,

        MediaBrowserIconSpotify = 200,
        MediaBrowserIconAmazon = 201,
        MediaBrowserIconTuneIn = 202,
        MediaBrowserIconSiriusXM = 203,
        MediaBrowserIconVTuner = 204,
        MediaBrowserIconTidal = 205,
        MediaBrowserIconAirable = 206,
        MediaBrowserIconDeezer = 207,
        MediaBrowserIconNapster = 208,
        MediaBrowserIconSoundCloud = 209,
        MediaBrowserIconRadioParadise = 210,
    };
    Q_ENUM(MediaBrowserIcon)

    MediaBrowserItem(const QString &id = QString(), const QString &displayName = QString(), bool browsable = false, bool executable = false);

    MediaBrowserIcon mediaIcon() const;
    void setMediaIcon(MediaBrowserIcon mediaIcon);

    int playCount() const;
    void setPlayCount(int playCount);
};

#endif // MEDIABROWSERITEM_H
