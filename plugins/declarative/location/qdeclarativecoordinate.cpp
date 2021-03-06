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

#include "qdeclarativecoordinate_p.h"
#include <qnumeric.h>
#include "qdeclarative.h"

QTM_BEGIN_NAMESPACE

/*!
    \qmlclass Coordinate

    \brief The Coordinate element holds various positional data, such as \l
    latitude, \l longitude and \l altitude.
    \inherits QObject

    \ingroup qml-location

    The Coordinate element is part of the \bold{QtMobility.location 1.1} module.
*/

QDeclarativeCoordinate::QDeclarativeCoordinate(QObject* parent)
        : QObject(parent) {}

QDeclarativeCoordinate::QDeclarativeCoordinate(const QGeoCoordinate &coordinate,
        QObject *parent)
        : QObject(parent),
        m_coordinate(coordinate) {}

QDeclarativeCoordinate::~QDeclarativeCoordinate() {}

void QDeclarativeCoordinate::setCoordinate(const QGeoCoordinate &coordinate)
{
    QGeoCoordinate previousCoordinate = m_coordinate;
    m_coordinate = coordinate;

    // Comparing two NotANumbers is false which is not wanted here
    if (coordinate.altitude() != previousCoordinate.altitude() &&
        !(qIsNaN(coordinate.altitude()) && qIsNaN(previousCoordinate.altitude()))) {
        emit altitudeChanged(m_coordinate.altitude());
    }
    if (coordinate.latitude() != previousCoordinate.latitude() &&
        !(qIsNaN(coordinate.latitude()) && qIsNaN(previousCoordinate.latitude()))) {
        emit latitudeChanged(m_coordinate.latitude());
    }
    if (coordinate.longitude() != previousCoordinate.longitude() &&
        !(qIsNaN(coordinate.longitude()) && qIsNaN(previousCoordinate.longitude()))) {
        emit longitudeChanged(m_coordinate.longitude());
    }
}

QGeoCoordinate QDeclarativeCoordinate::coordinate() const
{
    return m_coordinate;
}

/*!
    \qmlproperty double Coordinate::altitude

    This property holds the value of altitude (metres above sea level).
    If the property has not been set, its default value is zero.

*/

void QDeclarativeCoordinate::setAltitude(double altitude)
{
    if (m_coordinate.altitude() != altitude) {
        m_coordinate.setAltitude(altitude);
        emit altitudeChanged(m_coordinate.altitude());
    }
}

double QDeclarativeCoordinate::altitude() const
{
    return m_coordinate.altitude();
}

/*!
    \qmlproperty double Coordinate::longitude

    This property holds the longitude value of the geographical position
    (decimal degrees). A positive longitude indicates the Eastern Hemisphere,
    and a negative longitude indicates the Western Hemisphere
    If the property has not been set, its default value is zero.
*/

void QDeclarativeCoordinate::setLongitude(double longitude)
{
    if (m_coordinate.longitude() != longitude) {
        m_coordinate.setLongitude(longitude);
        emit longitudeChanged(m_coordinate.longitude());
    }
}

double QDeclarativeCoordinate::longitude() const
{
    return m_coordinate.longitude();
}

/*!
    \qmlproperty double Coordinate::latitude

    This property holds latitude value of the geographical position
    (decimal degrees). A positive latitude indicates the Northern Hemisphere,
    and a negative latitude indicates the Southern Hemisphere.
    If the property has not been set, its default value is zero.
*/

void QDeclarativeCoordinate::setLatitude(double latitude)
{
    if (m_coordinate.latitude() != latitude) {
        m_coordinate.setLatitude(latitude);
        emit latitudeChanged(latitude);
    }
}

double QDeclarativeCoordinate::latitude() const
{
    return m_coordinate.latitude();
}

qreal QDeclarativeCoordinate::distanceTo(QObject* coordinate)
{
    QDeclarativeCoordinate* coord = static_cast<QDeclarativeCoordinate*>(coordinate);
    return m_coordinate.distanceTo(coord->coordinate());
}

#include "moc_qdeclarativecoordinate_p.cpp"

QTM_END_NAMESPACE

