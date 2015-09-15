/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#include "vendorsresource.h"
#include "httprequest.h"
#include "loggingcategories.h"
#include "guhcore.h"

#include <QJsonDocument>

namespace guhserver {

VendorsResource::VendorsResource(QObject *parent) :
    RestResource(parent)
{
}

QString VendorsResource::name() const
{
    return "vendors";
}

HttpReply *VendorsResource::proccessRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    // /api/v1/vendors/{vendorId}/
    if (urlTokens.count() >= 4) {
        m_vendorId = VendorId(urlTokens.at(3));
        if (m_vendorId.isNull()) {
            qCWarning(dcRest) << "Could not parse VendorId:" << urlTokens.at(3);
            return createErrorReply(HttpReply::BadRequest);
        }
    }

    // check method
    HttpReply *reply;
    switch (request.method()) {
    case HttpRequest::Get:
        reply = proccessGetRequest(request, urlTokens);
        break;
    default:
        reply = createErrorReply(HttpReply::BadRequest);
        break;
    }
    return reply;
}

HttpReply *VendorsResource::proccessGetRequest(const HttpRequest &request, const QStringList &urlTokens)
{
    Q_UNUSED(request)

    // GET /api/v1/vendors
    if (urlTokens.count() == 3)
        return getVendors();

    // GET /api/v1/vendors/{vendorId}
    if (urlTokens.count() == 4)
        return getVendor(m_vendorId);

    return createErrorReply(HttpReply::NotImplemented);
}

HttpReply *VendorsResource::getVendors() const
{
    qCDebug(dcRest) << "Get vendors";
    HttpReply *reply = createSuccessReply();

    QVariantList vendorsList;
    foreach (const Vendor &vendor, GuhCore::instance()->supportedVendors()) {
        vendorsList.append(JsonTypes::packVendor(vendor));
    }
    reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
    reply->setPayload(QJsonDocument::fromVariant(vendorsList).toJson());
    return reply;
}

HttpReply *VendorsResource::getVendor(const VendorId &vendorId) const
{
    qCDebug(dcRest) << "Get vendor with id" << vendorId;
    foreach (const Vendor &vendor, GuhCore::instance()->supportedVendors()) {
        if (vendor.id() == vendorId) {
            HttpReply *reply = createSuccessReply();
            reply->setHeader(HttpReply::ContentTypeHeader, "application/json; charset=\"utf-8\";");
            reply->setPayload(QJsonDocument::fromVariant(JsonTypes::packVendor(vendor)).toJson());
            return reply;
        }
    }
    return createErrorReply(HttpReply::NotFound);
}

}

