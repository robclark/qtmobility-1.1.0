/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in 
** accordance with the Qt Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVEGEOMAPPIXMAPOBJECT_H
#define QDECLARATIVEGEOMAPPIXMAPOBJECT_H

#include "qdeclarativecoordinate_p.h"
#include "qgeomappixmapobject.h"

#include <QColor>
#include <QUrl>
#include <QNetworkReply>

QTM_BEGIN_NAMESPACE

class QDeclarativeGeoMapPixmapObject : public QGeoMapPixmapObject
{
    Q_OBJECT
    Q_ENUMS(Status)

    Q_PROPERTY(QDeclarativeCoordinate* coordinate READ declarativeCoordinate WRITE setDeclarativeCoordinate NOTIFY declarativeCoordinateChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

public:
    enum Status {
        Null,
        Ready,
        Loading,
        Error
    };

    QDeclarativeGeoMapPixmapObject();
    ~QDeclarativeGeoMapPixmapObject();

    QDeclarativeCoordinate* declarativeCoordinate();
    void setDeclarativeCoordinate(const QDeclarativeCoordinate *coordinate);

    QUrl source() const;
    void setSource(const QUrl &source);

    Status status() const;

Q_SIGNALS:
    void declarativeCoordinateChanged(const QDeclarativeCoordinate *coordinate);
    void sourceChanged(const QUrl &source);
    void statusChanged(QDeclarativeGeoMapPixmapObject::Status status);

private Q_SLOTS:
    void coordinateLatitudeChanged(double latitude);
    void coordinateLongitudeChanged(double longitude);
    void coordinateAltitudeChanged(double altitude);
    void finished();
    void error(QNetworkReply::NetworkError error);

private:
    void setStatus(const QDeclarativeGeoMapPixmapObject::Status status);
    void load();

    Status m_status;
    QDeclarativeCoordinate* m_coordinate;
    QUrl m_source;
    QNetworkReply *m_reply;

    Q_DISABLE_COPY(QDeclarativeGeoMapPixmapObject)
};

QTM_END_NAMESPACE

QML_DECLARE_TYPE(QTM_PREPEND_NAMESPACE(QDeclarativeGeoMapPixmapObject));

#endif
