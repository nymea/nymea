/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.guru>                 *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "guhavahibrowser.h"
#include "loggingcategories.h"

GuhAvahiBrowser::GuhAvahiBrowser(QObject *parent) : QObject(parent)
{
    m_browser = new ZConfServiceBrowser(this);

    connect(m_browser, &ZConfServiceBrowser::serviceEntryAdded, this, &GuhAvahiBrowser::onSeviceAdded);
    connect(m_browser, &ZConfServiceBrowser::serviceEntryRemoved, this, &GuhAvahiBrowser::onSeviceRemoved);
    connect(m_browser, &ZConfServiceBrowser::serviceBrowsingFinished, this, &GuhAvahiBrowser::onSeviceBrowsingFinished);
}

void GuhAvahiBrowser::browseService(const QString &serviceType)
{
    m_serviceEntries.clear();
    m_browser->browse(serviceType);
}

void GuhAvahiBrowser::onSeviceBrowsingFinished()
{

    emit avahiBrowseFinished(m_serviceEntries);
}

