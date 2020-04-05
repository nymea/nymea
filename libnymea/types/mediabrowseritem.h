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

#ifndef MEDIABROWSERITEM_H
#define MEDIABROWSERITEM_H

#include "browseritem.h"

class MediaBrowserItem: public BrowserItem
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
