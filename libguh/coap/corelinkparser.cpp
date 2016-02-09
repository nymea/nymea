/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

/*!
    \class CoreLinkParser
    \brief Provides an easy way to parse a CoRE link list.

    \ingroup coap
    \inmodule libguh

    \section2 Example

    \code
        if (reply->contentType == CoapPdu::ApplicationLink) {
            CoreLinkParser parser(reply->payload());

            foreach (const CoreLink &link, parser.links()) {
                qDebug() << link;
            }
        }
    \endcode

    \sa CoreLink

*/

#include "corelinkparser.h"

#include <QDebug>

/*! Constructs a CoRE link parser with the given \a parent. The given \a data should contain a CoRE link list from the discovery. */
CoreLinkParser::CoreLinkParser(const QByteArray &data, QObject *parent) :
    QObject(parent),
    m_data(data)
{
    QList<QByteArray> linkList = data.split(',');
    foreach (const QByteArray &linkLine, linkList) {
        qDebug() << "-------------------------------------";
        qDebug() << linkLine;
        QList<QByteArray> valueList = linkLine.split(';');
        CoreLink link;
        foreach (const QByteArray &value, valueList) {
            qDebug() << value;
            if (value.startsWith("<")) {
                link.setPath(QString(value.mid(1, value.length() - 2)));
            } else if (value.startsWith("rt=")) {
                link.setResourceType(QString(value.right(value.length() - 3)).remove('"'));
            } else if (value.startsWith("if=")) {
                link.setInterfaceDescription(QString(value.right(value.length() - 3)).remove('"'));
            } else if (value.startsWith("sz=")) {
                link.setMaximumSize(value.right(value.length() - 3).toInt());
            } else if (value.startsWith("ct=")) {
                link.setContentType(static_cast<CoapPdu::ContentType>(value.right(value.length() - 3).toUInt()));
            } else if (value.startsWith("title=")) {
                link.setTitle(QString(value.right(value.length() - 6)).remove('"'));
            } else if (value == "obs") {
                link.setObservable(true);
            }
        }
        qDebug() << endl << link;
        m_links.append(link);
    }
}

/*! Returns the parsed list of \l{CoreLink}{CoreLinks}.*/
QList<CoreLink> CoreLinkParser::links() const
{
    return m_links;
}


