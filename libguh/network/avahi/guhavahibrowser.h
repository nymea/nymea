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

#ifndef GUHAVAHIBROWSER_H
#define GUHAVAHIBROWSER_H

#include <QObject>

#include "zconfservicebrowser.h"

class GuhAvahiBrowser : public QObject
{
    Q_OBJECT
public:
    explicit GuhAvahiBrowser(QObject *parent = 0);

    void browseService(const QString &serviceType);

private:
    ZConfServiceBrowser *m_browser;
    QList<AvahiServiceEntry> m_serviceEntries;

signals:
    void avahiBrowseFinished(const QList<AvahiServiceEntry> &serviceEntries);

private slots:
    void onSeviceAdded(const QString &name);
    void onSeviceRemoved(const QString &name);
    void onSeviceBrowsingFinished();

};

#endif // GUHAVAHIBROWSER_H
