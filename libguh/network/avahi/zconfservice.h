/*
 *  This file is part of qtzeroconf. (c) 2012 Johannes Hilden
 *  https://github.com/johanneshilden/qtzeroconf
 *
 *  Modified: (C) 2016 Simon Stürz <stuerz.simon@gmail.com>
 *
 *  qtzeroconf is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of the
 *  License, or (at your option) any later version.
 *
 *  qtzeroconf is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
 *  Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with qtzeroconf; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef ZCONFSERVICE_H
#define ZCONFSERVICE_H

#include <QObject>
#include <arpa/inet.h>

class ZConfServicePrivate;
class ZConfService : public QObject
{
    Q_OBJECT

public:
    explicit ZConfService(QObject *parent = 0);
    ~ZConfService();

    bool isValid() const;
    QString errorString() const;

signals:
    void entryGroupEstablished();
    void entryGroupNameCollision();
    void entryGroupFailure();

public slots:
    void registerService(QString name, in_port_t port, QString type = "_http._tcp");
    void resetService();

protected:
    ZConfServicePrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE(ZConfService)
};

#endif // ZCONFSERVICE_H
