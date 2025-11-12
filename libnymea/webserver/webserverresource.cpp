/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2025, nymea GmbH
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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "webserverresource.h"
#include "loggingcategories.h"

#include <QFile>

WebServerResource::WebServerResource(const QString &basePath, QObject *parent)
    : QObject{parent},
    m_basePath{basePath}
{

}

QString WebServerResource::basePath() const
{
    return m_basePath;
}

bool WebServerResource::enabled() const
{
    return m_enabled;
}

void WebServerResource::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    qCDebug(dcWebServer()) << "The resource" << m_basePath << "is now" << (enabled ? "enabled" : "disabled");
    m_enabled = enabled;
    emit enabledChanged(m_enabled);
}

HttpReply *WebServerResource::createFileReply(const QString fileName)
{
    qCDebug(dcWebServer()) << "Create file reply for" << fileName;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(dcWebServer()) << "Unable to generate file reply. The file" << fileName << "could not be opened. Respond with 403 Forbidden.";
        return HttpReply::createErrorReply(HttpReply::Forbidden);
    }

    HttpReply *reply = HttpReply::createSuccessReply();

    // Check content type
    if (file.fileName().endsWith(".html")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "text/html; charset=\"utf-8\";");
    } else if (file.fileName().endsWith(".css")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "text/css; charset=\"utf-8\";");
    } else if (file.fileName().endsWith(".pdf")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "application/pdf");
    } else if (file.fileName().endsWith(".js")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "text/javascript; charset=\"utf-8\";");
    } else if (file.fileName().endsWith(".ttf")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "application/x-font-ttf");
    } else if (file.fileName().endsWith(".eot")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "application/vnd.ms-fontobject");
    } else if (file.fileName().endsWith(".woff")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "application/x-font-woff");
    } else if (file.fileName().endsWith(".jpg") || file.fileName().endsWith(".jpeg")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "image/jpeg");
    } else if (file.fileName().endsWith(".png") || file.fileName().endsWith(".PNG")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "image/png");
    } else if (file.fileName().endsWith(".ico")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "image/x-icon");
    } else if (file.fileName().endsWith(".svg")) {
        reply->setHeader(HttpReply::ContentTypeHeader, "image/svg+xml; charset=\"utf-8\";");
    }

    reply->setPayload(file.readAll());
    return reply;
}
