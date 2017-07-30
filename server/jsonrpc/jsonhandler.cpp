/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

/*!
    \class guhserver::JsonHandler
    \brief This class represents an interface for developing a handler for the JSON-RPC API.

    \ingroup json
    \inmodule core

    \sa JsonRPCServer, JsonReply
*/

/*! \fn QString guhserver::JsonHandler::name() const;
    Pure virtual method for a JSON RPC handler. Returns the namespace of the handler.
*/

/*! \fn void guhserver::JsonHandler::asyncReply(int id, const QVariantMap &params);
    This signal will be emitted when a reply with the given \a id and \a params is finished.
*/


#include "jsonhandler.h"
#include "loggingcategories.h"

#include <QMetaMethod>
#include <QDebug>
#include <QRegExp>

namespace guhserver {

/*! Constructs a new \l JsonHandler with the given \a parent. */
JsonHandler::JsonHandler(QObject *parent) :
    QObject(parent)
{
}

/*! Returns a map with all supported methods, notifications and types for the given meta \a type. */
QVariantMap JsonHandler::introspect(QMetaMethod::MethodType type)
{
    QVariantMap data;
    for (int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);

        if (method.methodType() != type) {
            continue;
        }

        switch (method.methodType()) {
        case QMetaMethod::Method: {
            if (!m_descriptions.contains(method.name()) || !m_params.contains(method.name()) || !m_returns.contains(method.name())) {
                continue;
            }
            qCDebug(dcJsonRpc) << "got method" << method.name();
            QVariantMap methodData;
            methodData.insert("description", m_descriptions.value(method.name()));
            methodData.insert("params", m_params.value(method.name()));
            methodData.insert("returns", m_returns.value(method.name()));
            data.insert(name() + "." + method.name(), methodData);
            break;
        }
        case QMetaMethod::Signal: {
            if (!m_descriptions.contains(method.name()) || !m_params.contains(method.name())) {
                continue;
            }
            if (QString(method.name()).contains(QRegExp("^[A-Z]"))) {
                qCDebug(dcJsonRpc) << "got signal" << method.name();
                QVariantMap methodData;
                methodData.insert("description", m_descriptions.value(method.name()));
                methodData.insert("params", m_params.value(method.name()));
                data.insert(name() + "." + method.name(), methodData);
            }
            break;
        default:
            ;;// Nothing to do for slots
        }
        }
    }
    return data;
}

/*! Returns true if this \l JsonHandler has a method with the given \a methodName.*/
bool JsonHandler::hasMethod(const QString &methodName)
{
    return m_descriptions.contains(methodName) && m_params.contains(methodName) && m_returns.contains(methodName);
}

/*! Validates the given \a params for the given \a methodName. Returns the error string and false if
    the params are not valid. */
QPair<bool, QString> JsonHandler::validateParams(const QString &methodName, const QVariantMap &params)
{
    QVariantMap paramTemplate = m_params.value(methodName);
    return JsonTypes::validateMap(paramTemplate, params);
}

/*! Validates the given \a returns for the given \a methodName. Returns the error string and false if
    the params are not valid. */
QPair<bool, QString> JsonHandler::validateReturns(const QString &methodName, const QVariantMap &returns)
{
    QVariantMap returnsTemplate = m_returns.value(methodName);
    return JsonTypes::validateMap(returnsTemplate, returns);
}


/*! Sets the \a description of the method with the given \a methodName. */
void JsonHandler::setDescription(const QString &methodName, const QString &description)
{
    for(int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.name() == methodName) {
            m_descriptions.insert(methodName, description);
            return;
        }
    }
    qCWarning(dcJsonRpc) << "Cannot set description. No such method:" << methodName;
}

/*! Sets the \a params of the method with the given \a methodName. */
void JsonHandler::setParams(const QString &methodName, const QVariantMap &params)
{
    for(int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.name() == methodName) {
            m_params.insert(methodName, params);
            return;
        }
    }
    qCWarning(dcJsonRpc) << "Cannot set params. No such method:" << methodName;
}

/*! Sets the \a returns of the method with the given \a methodName. */
void JsonHandler::setReturns(const QString &methodName, const QVariantMap &returns)
{
    for(int i = 0; i < metaObject()->methodCount(); ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.name() == methodName) {
            m_returns.insert(methodName, returns);
            return;
        }
    }
    qCWarning(dcJsonRpc) << "Cannot set returns. No such method:" << methodName;
}

/*! Returns the pointer to a new \l{JsonReply} with the given \a data. */
JsonReply *JsonHandler::createReply(const QVariantMap &data) const
{
    return JsonReply::createReply(const_cast<JsonHandler*>(this), data);
}

/*! Returns the pointer to an asynchronous new \l{JsonReply} with the given \a method. */
JsonReply* JsonHandler::createAsyncReply(const QString &method) const
{
    return JsonReply::createAsyncReply(const_cast<JsonHandler*>(this), method);
}

/*! Returns the formated error map for the given \a status.
 *
 *  \sa DeviceManager::DeviceError
 */
QVariantMap JsonHandler::statusToReply(DeviceManager::DeviceError status) const
{
    QVariantMap returns;
    returns.insert("deviceError", JsonTypes::deviceErrorToString(status));
    return returns;
}

/*! Returns the formated error map for the given \a status.
 *
 *  \sa RuleEngine::RuleError
 */
QVariantMap JsonHandler::statusToReply(RuleEngine::RuleError status) const
{
    QVariantMap returns;
    returns.insert("ruleError", JsonTypes::ruleErrorToString(status));
    return returns;
}

/*! Returns the formated error map for the given \a status.
 *
 *  \sa Logging::LoggingError
 */
QVariantMap JsonHandler::statusToReply(Logging::LoggingError status) const
{
    QVariantMap returns;
    returns.insert("loggingError", JsonTypes::loggingErrorToString(status));
    return returns;
}

/*! Returns the formated error map for the given \a status. */
QVariantMap JsonHandler::statusToReply(GuhConfiguration::ConfigurationError status) const
{
    QVariantMap returns;
    returns.insert("configurationError", JsonTypes::configurationErrorToString(status));
    return returns;
}

/*! Returns the formated error map for the given \a status. */
QVariantMap JsonHandler::statusToReply(NetworkManager::NetworkManagerError status) const
{
    QVariantMap returns;
    returns.insert("networkManagerError", JsonTypes::networkManagerErrorToString(status));
    return returns;
}


/*!
    \class guhserver::JsonReply
    \brief This class represents a reply for the JSON-RPC API request.

    \ingroup json
    \inmodule core

    \sa JsonHandler, JsonRPCServer
*/

/*! \enum guhserver::JsonReply::Type

    This enum type specifies the type of a JsonReply.

    \value TypeSync
        The response is synchronous.
    \value TypeAsync
        The response is asynchronous.
*/

/*! \fn void guhserver::JsonReply::finished();
    This signal will be emitted when a JsonReply is finished. A JsonReply is finished when
    the response is ready or then the reply timed out.
*/



/*! Constructs a new \l JsonReply with the given \a type, \a handler, \a method and \a data. */
JsonReply::JsonReply(Type type, JsonHandler *handler, const QString &method, const QVariantMap &data):
    m_type(type),
    m_data(data),
    m_handler(handler),
    m_method(method),
    m_timedOut(false)
{
    connect(&m_timeout, &QTimer::timeout, this, &JsonReply::timeout);
}

/*! Returns the pointer to a new \l{JsonReply} for the given \a handler and \a data. */
JsonReply *JsonReply::createReply(JsonHandler *handler, const QVariantMap &data)
{
    return new JsonReply(TypeSync, handler, QString(), data);
}

/*! Returns the pointer to a new asynchronous \l{JsonReply} for the given \a handler and \a method. */
JsonReply *JsonReply::createAsyncReply(JsonHandler *handler, const QString &method)
{
    return new JsonReply(TypeAsync, handler, method);
}

/*! Returns the type of this \l{JsonReply}.*/
JsonReply::Type JsonReply::type() const
{
    return m_type;
}

/*! Returns the data of this \l{JsonReply}.*/
QVariantMap JsonReply::data() const
{
    return m_data;
}

/*! Sets the \a data of this \l{JsonReply}.*/
void JsonReply::setData(const QVariantMap &data)
{
    m_data = data;
}

/*! Returns the handler of this \l{JsonReply}.*/
JsonHandler *JsonReply::handler() const
{
    return m_handler;
}

/*! Returns the method of this \l{JsonReply}.*/
QString JsonReply::method() const
{
    return m_method;
}

/*! Returns the client ID of this \l{JsonReply}.*/
QUuid JsonReply::clientId() const
{
    return m_clientId;
}

/*! Sets the \a clientId of this \l{JsonReply}.*/
void JsonReply::setClientId(const QUuid &clientId)
{
    m_clientId = clientId;
}

/*! Returns the command ID of this \l{JsonReply}.*/
int JsonReply::commandId() const
{
    return m_commandId;
}

/*! Returns the \a commandId of this \l{JsonReply}.*/
void JsonReply::setCommandId(int commandId)
{
    m_commandId = commandId;
}

/*! Start the timeout timer for this \l{JsonReply}. The default timeout is 10 seconds. */
void JsonReply::startWait()
{
    m_timeout.start(15000);
}

void JsonReply::timeout()
{
    m_timedOut = true;
    emit finished();
}

/*! Returns true if this \l{JsonReply} timed out.*/
bool JsonReply::timedOut() const
{
    return m_timedOut;
}

}
