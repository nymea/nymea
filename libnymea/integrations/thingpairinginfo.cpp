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

#include "thingpairinginfo.h"
#include "thingmanager.h"

#include <QTimer>

Q_DECLARE_LOGGING_CATEGORY(dcIntegrations)

ThingPairingInfo::ThingPairingInfo(const PairingTransactionId &pairingTransactionId, const ThingClassId &thingClassId, const ThingId &thingId, const QString &deviceName, const ParamList &params, const ThingId &parentId, ThingManager *parent, bool reconfigure, quint32 timeout):
    QObject(parent),
    m_transactionId(pairingTransactionId),
    m_thingClassId(thingClassId),
    m_thingId(thingId),
    m_thingName(deviceName),
    m_params(params),
    m_parentId(parentId),
    m_reconfigure(reconfigure)
{
    connect(this, &ThingPairingInfo::finished, this, &ThingPairingInfo::deleteLater, Qt::QueuedConnection);

    // TODO 2.0: Create a base class and move this finished() of all Info classes logic handling there
    // That will badly break the ABI, so copy/pasting this for now.
    if (timeout > 0) {
        QTimer::singleShot(timeout, this, [this] {
            // It can happen that a plugin calls finish() in a slot which normally would be dispatched before the timeout
            // but due to high system load the slot is invoked only after the timeout. This in turn would cause Qt to also queue up
            // this timeout slot and by the time the system processes slots, the plugin comes in first and we'd fire an aborted()
            // signal in the plugin after it called finish(), potentially badly breaking the plugin as a plugin developer would not
            // expect this to happen. So we'll have to verify here that the plugin did not finish() by now before aborting.
            if (!m_finished) {
                emit aborted();
                // Also it could happen the the plugin calls finish() *in* the aborted() signal, in which case
                // we don't want to call finish() here now to not print a warning about duplicate finished calls
                if (!m_finished) {
                    finish(Thing::ThingErrorTimeout);
                }
            }
        });
    }
}

PairingTransactionId ThingPairingInfo::transactionId() const
{
    return m_transactionId;
}

ThingClassId ThingPairingInfo::thingClassId() const
{
    return m_thingClassId;
}

ThingId ThingPairingInfo::thingId() const
{
    return m_thingId;
}

QString ThingPairingInfo::thingName() const
{
    return m_thingName;
}

ParamList ThingPairingInfo::params() const
{
    return m_params;
}

ThingId ThingPairingInfo::parentId() const
{
    return m_parentId;
}

QUrl ThingPairingInfo::oAuthUrl() const
{
    return m_oAuthUrl;
}

bool ThingPairingInfo::isReconfigure() const
{
    return m_reconfigure;
}

void ThingPairingInfo::setOAuthUrl(const QUrl &oAuthUrl)
{
    m_oAuthUrl = oAuthUrl;
}

Thing::ThingError ThingPairingInfo::status() const
{
    return m_status;
}

QString ThingPairingInfo::displayMessage() const
{
    return m_displayMessage;
}

QString ThingPairingInfo::translatedDisplayMessage(const QLocale &locale) const
{
    if (!m_thingManager) {
        return m_displayMessage;
    }
    ThingClass thingClass = m_thingManager->findThingClass(m_thingClassId);
    return m_thingManager->translate(thingClass.pluginId(), m_displayMessage.toUtf8(), locale);
}

void ThingPairingInfo::finish(Thing::ThingError status, const QString &displayMessage)
{
    if (m_finished) {
        qCWarning(dcIntegrations()) << "ThingPairingInfo::finish() called on an already finished object.";
        return;
    }
    m_finished = true;
    m_status = status;
    m_displayMessage = displayMessage;
    staticMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}

