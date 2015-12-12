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

#ifndef COAPTESTS_H
#define COAPTESTS_H

#include <QUrl>
#include <QObject>
#include <QHostInfo>
#include <QHostAddress>

#include <QSignalSpy>
#include <QtTest>

#include "coap/coap.h"
#include "coap/coappdu.h"
#include "coap/coapreply.h"
#include "coap/corelinkparser.h"

class CoapTests : public QObject
{
    Q_OBJECT

public:
    explicit CoapTests(QObject *parent = 0);

private:
    Coap *m_coap;
    QByteArray m_uploadData;

private slots:
    void invalidUrl_data();
    void invalidUrl();

    void invalidScheme();

    void ping();
    void hello();
    void broken();
    void query();
    void subPath();
    void extendedOptionLength();

    void specialCharacters();

    void extendedDelta_data();
    void extendedDelta();

    void secret();
    void separated();

    void deleteResource();
    void post();
    void put();

    void jsonMessage();

    void largeDownload();
    void largeCreate();
    void largeUpdate();

    void multipleCalls();

    void coreLinkParser();

};

#endif // COAPTESTS_H
