/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#ifndef HEATPUMP_H
#define HEATPUMP_H

#include <QObject>
#include <QHostAddress>


#include "coap/coap.h"
#include "coap/coapreply.h"
#include "coap/coaprequest.h"

class HeatPump : public QObject
{
    Q_OBJECT
public:
    explicit HeatPump(QHostAddress address, QObject *parent = 0);

    QHostAddress address() const;
    bool reachable() const;
    void setSgMode(const int &sgMode);

private:
    QHostAddress m_address;
    bool m_reachable;
    int m_sgMode;

    Coap *m_coap;

    QList<CoapReply *> m_discoverReplies;
    QList<CoapReply *> m_sgModeReplies;

    void setReachable(const bool &reachable);

private slots:
    void onReplyFinished(CoapReply *reply);

signals:
    void reachableChanged();


};

#endif // HEATPUMP_H
