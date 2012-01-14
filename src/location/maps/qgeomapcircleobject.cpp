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

#include "qgeomapcircleobject.h"
#include "qgeomapcircleobject_p.h"

#include "qgeocoordinate.h"

#include <QDebug>

QTM_BEGIN_NAMESPACE

/*!
    \class QGeoMapCircleObject
    \brief The QGeoMapCircleObject class is a QGeoMapObject used to draw the region
    within a given distance of a coordinate.

    \inmodule QtLocation

    \ingroup maps-mapping-objects

    The circle is specified by either a valid QGeoBoundingCircle instance or a
    valid QGeoCoordinate instance and a qreal with value greater than 0.0,
    which represent the center of the circle and the radius of the circle in
    metres respectively.

    The circle may appear as an ellipse on maps which use the Mercator
    projection. This is done so that the circle accurately covers all points at
    a distance of the radius or less from the center.
*/

/*!
    Constructs a new circle object.
*/
QGeoMapCircleObject::QGeoMapCircleObject()
    : d_ptr(new QGeoMapCircleObjectPrivate()) {}

/*!
    Constructs a new circle object based on the circle \a circle.
*/
QGeoMapCircleObject::QGeoMapCircleObject(const QGeoBoundingCircle &circle)
    : d_ptr(new QGeoMapCircleObjectPrivate())
{
    d_ptr->circle = circle;
}

/*!
    Constructs a new circle object with a center at coordinate \a center
    and a radius in meters of \a radius.
*/
QGeoMapCircleObject::QGeoMapCircleObject(const QGeoCoordinate &center, qreal radius)
    : d_ptr(new QGeoMapCircleObjectPrivate())
{
    d_ptr->circle = QGeoBoundingCircle(center, radius);
}

/*!
    Destroys this circle object.
*/
QGeoMapCircleObject::~QGeoMapCircleObject()
{
    delete d_ptr;
}

/*!
    \reimp
*/
QGeoMapObject::Type QGeoMapCircleObject::type() const
{
    return QGeoMapObject::CircleType;
}

/*!
    \property QGeoMapCircleObject::pen
    \brief This property holds the pen that will be used to draw this object.

    The pen is used to draw an outline around the circle. The circle is
    filled using the QGeoMapCircleObject::brush property.

    The pen will be treated as a cosmetic pen, which means that the width
    of the pen will be independent of the zoom level of the map.
*/
void QGeoMapCircleObject::setPen(const QPen &pen)
{
    QPen newPen = pen;
    newPen.setCosmetic(true);

    if (d_ptr->pen == newPen)
        return;

    d_ptr->pen = newPen;
    emit penChanged(d_ptr->pen);
}

QPen QGeoMapCircleObject::pen() const
{
    return d_ptr->pen;
}

/*!
    \property QGeoMapCircleObject::brush
    \brief This property holds the brush that will be used to draw this object.

    The brush is used to fill in circle.

    The outline around the perimeter of the circle is drawn using the
    QGeoMapCircleObject::pen property.
*/
void QGeoMapCircleObject::setBrush(const QBrush &brush)
{
    if (d_ptr->brush != brush) {
        d_ptr->brush = brush;
        emit brushChanged(d_ptr->brush);
    }
}

QBrush QGeoMapCircleObject::brush() const
{
    return d_ptr->brush;
}

/*!
    Returns a QGeoBoundingCircle instance which corresponds to the circle that
    will be drawn by this object.

    This is equivalent to
    \code
        QGeoMapCircleObject *object;
        // setup object
        QGeoBoundingCircle(object->center(), object->radius());
    \endcode
*/
QGeoBoundingCircle QGeoMapCircleObject::circle() const
{
    return d_ptr->circle;
}

/*!
    Sets the circle that will be drawn by this object to \a circle.

    This is equivalent to
    \code
        QGeoMapCircleObject *object;
        // setup object
        object->setCenter(circle.center());
        object->setRadius(circle.radius());
    \endcode
*/
void QGeoMapCircleObject::setCircle(const QGeoBoundingCircle &circle)
{
    QGeoBoundingCircle oldCircle = d_ptr->circle;

    if (oldCircle == circle)
        return;

    d_ptr->circle = circle;

    if (oldCircle.center() != d_ptr->circle.center())
        emit centerChanged(d_ptr->circle.center());

    if (oldCircle.radius() != d_ptr->circle.radius())
        emit radiusChanged(d_ptr->circle.radius());
}

/*!
    \property QGeoMapCircleObject::center

    \brief This property holds the coordinate of the center of the circle to be
    drawn by this circle object.

    The default value of this property is an invalid coordinate.  While the
    value of this property is invalid the circle object will not be displayed.
*/
void QGeoMapCircleObject::setCenter(const QGeoCoordinate &center)
{
    if (d_ptr->circle.center() != center) {
        d_ptr->circle.setCenter(center);
        emit centerChanged(center);
    }
}

QGeoCoordinate QGeoMapCircleObject::center() const
{
    return d_ptr->circle.center();
}

/*!
    \property QGeoMapCircleObject::radius
    \brief This property holds the radius in metres of the circle that will be
    drawn by this circle object.

    The default value of this property is -1.0. While the value of this
    property is not greater than 0 the circle object will not be displayed.
*/
void QGeoMapCircleObject::setRadius(qreal radius)
{
    if (d_ptr->circle.radius() != radius) {
        d_ptr->circle.setRadius(radius);
        emit radiusChanged(radius);
    }
}

qreal QGeoMapCircleObject::radius() const
{
    return d_ptr->circle.radius();
}

/*!
\fn void QGeoMapCircleObject::centerChanged(const QGeoCoordinate &center)

    This signal is emitted when the center of the circle object has 
    changed.

    The new value is \a center.
*/

/*!
\fn void QGeoMapCircleObject::radiusChanged(qreal radius)

    This signal is emitted when the radius of the circle object has 
    changed.

    The new value is \a radius.
*/

/*!
\fn void QGeoMapCircleObject::penChanged(const QPen &pen)

    This signal is emitted when the pen used to draw the edge of 
    the circle object has changed.

    The new value is \a pen.
*/

/*!
\fn void QGeoMapCircleObject::brushChanged(const QBrush &brush)

    This signal is emitted when the brush used to fill the inside of 
    the circle object has changed.

    The new value is \a brush.
*/

/*******************************************************************************
*******************************************************************************/

QGeoMapCircleObjectPrivate::QGeoMapCircleObjectPrivate()
{
    pen.setCosmetic(true);
}

QGeoMapCircleObjectPrivate::~QGeoMapCircleObjectPrivate() {}

#include "moc_qgeomapcircleobject.cpp"

QTM_END_NAMESPACE

