/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2016 Simon Stuerz <simon.stuerz@guh.guru>           *
 *                                                                         *
 *  This file is part of QtCoap.                                           *
 *                                                                         *
 *  QtCoap is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 3 of the License.                *
 *                                                                         *
 *  QtCoap is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with QtCoap. If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef COREPARSER_H
#define COREPARSER_H

#include <QObject>

#include "corelink.h"

// Constrained RESTful Environments (CoRE) Link Format  : http://tools.ietf.org/html/rfc6690

class CoreLinkParser : public QObject
{
    Q_OBJECT
public:
    explicit CoreLinkParser(const QByteArray &data, QObject *parent = 0);

    QList<CoreLink> links() const;

private:
    QByteArray m_data;
    QList<CoreLink> m_links;
};

#endif // COREPARSER_H
