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

#include "heatpump.h"
#include "coap/corelinkparser.h"

#include "extern-plugininfo.h"

HeatPump::HeatPump(QHostAddress address, QObject *parent) :
    QObject(parent),
    m_address(address),
    m_reachable(false),
    m_sgMode(1)
{
    m_coap = new Coap(this);

    connect(m_coap, SIGNAL(replyFinished(CoapReply*)), this, SLOT(onReplyFinished(CoapReply*)));

    QUrl url;
    url.setScheme("coap");
    url.setHost(m_address.toString());
    url.setPath("/.well-known/core");

    qCDebug(dcAwattar) << "Discover pump resources on" << url.toString();
    m_discoverReplies.append(m_coap->get(CoapRequest(url)));
}

QHostAddress HeatPump::address() const
{
    return m_address;
}

bool HeatPump::reachable() const
{
    return m_reachable;
}

void HeatPump::setSgMode(const int &sgMode)
{
    QUrl url;
    url.setScheme("coap");
    url.setHost(m_address.toString());
    url.setPath("/a/sg_mode");

    m_sgModeReplies.append(m_coap->post(CoapRequest(url), QByteArray::number(sgMode)));
}

void HeatPump::setLed(const bool &power)
{
    QUrl url;
    url.setScheme("coap");
    url.setHost(m_address.toString());
    url.setPath("/a/led");

    if (power) {
        m_ledReplies.append(m_coap->post(CoapRequest(url), "mode=on"));
    } else {
        m_ledReplies.append(m_coap->post(CoapRequest(url), "mode=off"));
    }
}

void HeatPump::setReachable(const bool &reachable)
{
    m_reachable = reachable;
    emit reachableChanged();
}

void HeatPump::onReplyFinished(CoapReply *reply)
{
    if (m_discoverReplies.contains(reply)) {
        m_discoverReplies.removeAll(reply);

        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcAwattar()) << "CoAP resource discovery reply error" << reply->errorString();
            setReachable(false);
            reply->deleteLater();
            return;
        }

        qCDebug(dcAwattar) << "Discovered successfully the resources";
        CoreLinkParser parser(reply->payload());
        foreach (const CoreLink &link, parser.links()) {
            qCDebug(dcAwattar) << link << endl;
        }

    } else if (m_sgModeReplies.contains(reply)) {
        m_sgModeReplies.removeAll(reply);

        if (reply->error() != CoapReply::NoError) {
            if (reachable())
                qCWarning(dcAwattar()) << "CoAP sg-mode reply error" << reply->errorString();

            setReachable(false);
            reply->deleteLater();
            return;
        }

        if (!reachable())
            qCDebug(dcAwattar) << "Set sg-mode successfully.";

    } else if (m_ledReplies.contains(reply)) {
        m_ledReplies.removeAll(reply);

        if (reply->error() != CoapReply::NoError) {
            if (reachable())
                qCWarning(dcAwattar()) << "CoAP set led power reply error" << reply->errorString();

            setReachable(false);
            reply->deleteLater();
            return;
        }

        qCDebug(dcAwattar) << "Set led power successfully.";

    } else {
        // unhandled reply
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcAwattar()) << "CoAP reply error" << reply->errorString();
            setReachable(false);
            reply->deleteLater();
            return;
        }

        qCDebug(dcAwattar) << reply;
    }

    // the reply had no error until now, so make sure the resource is reachable
    setReachable(true);
    reply->deleteLater();
}
