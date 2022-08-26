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

#include "browseractioninfo.h"
#include "thingmanager.h"

#include <QTimer>

Q_DECLARE_LOGGING_CATEGORY(dcIntegrations)

BrowserActionInfo::BrowserActionInfo(Thing *thing, ThingManager *thingManager, const BrowserAction &browserAction, QObject *parent, quint32 timeout):
    QObject (parent),
    m_thing(thing),
    m_browserAction(browserAction),
    m_thingManager(thingManager)
{
    connect(this, &BrowserActionInfo::finished, this, &BrowserActionInfo::deleteLater, Qt::QueuedConnection);

    if (timeout > 0) {
        QTimer::singleShot(timeout, this, [this] {
            emit aborted();
            if (!m_finished) {
                finish(Thing::ThingErrorTimeout);
            }
        });
    }
}

Thing *BrowserActionInfo::thing() const
{
    return m_thing;
}

BrowserAction BrowserActionInfo::browserAction() const
{
    return m_browserAction;
}

bool BrowserActionInfo::isFinished() const
{
    return m_finished;
}

Thing::ThingError BrowserActionInfo::status() const
{
    return m_status;
}

QString BrowserActionInfo::displayMessage() const
{
    return m_displayMessage;
}

QString BrowserActionInfo::translatedDisplayMessage(const QLocale &locale)
{
    if (!m_thingManager || !m_thing) {
        return m_displayMessage;
    }

    return m_thingManager->translate(m_thing->pluginId(), m_displayMessage.toUtf8(), locale);
}

void BrowserActionInfo::finish(Thing::ThingError status, const QString &displayMessage)
{
    if (m_finished) {
        qCWarning(dcIntegrations()) << "BrowserActionInfo::finish() called on an already finished object.";
        return;
    }
    m_finished = true;
    m_status = status;
    m_displayMessage = displayMessage;
    staticMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}
