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

/*!
    \class CoreLink
    \brief Represents a link of a CoRE link format.

    \ingroup coap-group
    \inmodule libnymea

    This class represents a Constrained RESTful Environments (CoRE) Link format according to the
    \l{http://tools.ietf.org/html/rfc6690}{RFC6690} specification.

*/


#include "corelink.h"

#include <QMetaEnum>

/*! Constructs a \l{CoreLink}. */
CoreLink::CoreLink() :
    m_contentType(CoapPdu::TextPlain),
    m_maximumSize(-1),
    m_observable(false)
{
}

/*! Returns the path of this \l{CoreLink}. */
QString CoreLink::path() const
{
    return m_path;
}

/*! Sets the \a path of this \l{CoreLink}. */
void CoreLink::setPath(const QString &path)
{
    m_path = path;
}

/*! Returns the title of this \l{CoreLink}. */
QString CoreLink::title() const
{
    return m_title;
}

/*! Sets the \a title of this \l{CoreLink}. */
void CoreLink::setTitle(const QString &title)
{
    m_title = title;
}

/*! Returns the resource type of this \l{CoreLink}. */
QString CoreLink::resourceType() const
{
    return m_resourceType;
}

/*! Sets the resource type of this \l{CoreLink} to the given \a resourceType. */
void CoreLink::setResourceType(const QString &resourceType)
{
    m_resourceType = resourceType;
}

/*! Returns the interface description of this \l{CoreLink}. */
QString CoreLink::interfaceDescription() const
{
    return m_interfaceDescription;
}

/*! Sets the interface description of this \l{CoreLink} to the given \a interfaceDescription. */
void CoreLink::setInterfaceDescription(const QString &interfaceDescription)
{
    m_interfaceDescription = interfaceDescription;
}

/*! Returns the l{CoapPdu::ContentType} of this \l{CoreLink}. */
CoapPdu::ContentType CoreLink::contentType() const
{
    return m_contentType;
}

/*! Sets the l{CoapPdu::ContentType} of this \l{CoreLink} to the given \a contentType. */
void CoreLink::setContentType(const CoapPdu::ContentType &contentType)
{
    m_contentType = contentType;
}

/*! Returns the maximum payload size of this \l{CoreLink}. */
int CoreLink::maximumSize() const
{
    return m_maximumSize;
}

/*! Sets the maximum payload size of this \l{CoreLink} to the given \a maximumSize. */
void CoreLink::setMaximumSize(const int &maximumSize)
{
    m_maximumSize = maximumSize;
}

/*! Returns true if this \l{CoreLink} is observable. */
bool CoreLink::observable() const
{
    return m_observable;
}

/*! Sets this \l{CoreLink} \a observable. */
void CoreLink::setObservable(const bool &observable)
{
    m_observable = observable;
}

/*! Writes the data of the given \a link to \a dbg.

    \sa CoreLink
*/
QDebug operator<<(QDebug debug, const CoreLink &link)
{
    QDebugStateSaver saver(debug);
    const QMetaObject &metaObject = CoapPdu::staticMetaObject;
    QMetaEnum contentTypeEnum = metaObject.enumerator(metaObject.indexOfEnumerator("ContentType"));
    debug.nospace() << "CoapLink(" << link.path() << ")" << endl;

    if (!link.title().isEmpty())
        debug.nospace() << "  Title: " << link.title() << endl;

    debug.nospace() << "  Resource type: " << link.resourceType() << endl;
    debug.nospace() << "  Content type: " << contentTypeEnum.valueToKey(link.contentType()) << endl;

    if (link.observable())
        debug.nospace() << "  Observable: " << link.observable() << endl;

    if (!link.interfaceDescription().isEmpty())
        debug.nospace() << "  Interface description: " << link.interfaceDescription() << endl;

    if (link.maximumSize() >= 0)
        debug.nospace() << "  Maximum size: " << link.maximumSize() << endl;

    return debug;
}
