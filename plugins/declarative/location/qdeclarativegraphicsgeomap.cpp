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

#include "qdeclarativegraphicsgeomap_p.h"

#include "qdeclarativecoordinate_p.h"
#include "qdeclarativegeoserviceprovider_p.h"

#include <qgeoserviceprovider.h>
#include <qgeomappingmanager.h>
#include <qgeomapdata.h>
#include <qgeomapobject.h>

QTM_BEGIN_NAMESPACE

/*!
    \qmlclass Map

    \brief The Map element displays a map.
    \inherits QDeclarativeItem

    \ingroup qml-location-maps

    The Map element can be used be used to display a map of the world.  The 
    bulk of the functionality is provided by a mapping plugin described 
    by the Plugin element associated with the Map.

    Various map objects can be added to the map.  These map objects are 
    specified in terms of coordinates and metres.


    The Map element is part of the \bold{QtMobility.location 1.1} module.
*/
QDeclarativeGraphicsGeoMap::QDeclarativeGraphicsGeoMap(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
    serviceProvider_(0),
    mapData_(0)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);

    center_ = new QDeclarativeCoordinate(this);

    connect(center_,
            SIGNAL(latitudeChanged(double)),
            this,
            SLOT(centerLatitudeChanged(double)));
    connect(center_,
            SIGNAL(longitudeChanged(double)),
            this,
            SLOT(centerLongitudeChanged(double)));
    connect(center_,
            SIGNAL(altitudeChanged(double)),
            this,
            SLOT(centerAltitudeChanged(double)));

    center_->setCoordinate(QGeoCoordinate(-27.0, 153.0));
    zoomLevel_ = 8;
    size_ = QSizeF(100.0, 100.0);
    
}

QDeclarativeGraphicsGeoMap::~QDeclarativeGraphicsGeoMap()
{
    qDeleteAll(objects_);

    if (mapData_)
        delete mapData_;

    if (serviceProvider_)
        delete serviceProvider_;
}

void QDeclarativeGraphicsGeoMap::paint(QPainter *painter,
                                       const QStyleOptionGraphicsItem *option,
                                       QWidget * /*widget*/)
{
    if (mapData_) {
        mapData_->paint(painter, option);
    }
}

void QDeclarativeGraphicsGeoMap::geometryChanged(const QRectF &newGeometry,
                                                 const QRectF & /*oldGeometry*/)
{
    setSize(newGeometry.size());
}

/*!
    \qmlproperty Plugin Map::plugin

    This property holds the plugin which provides the mapping functionality.

    This is write-once property.  Once the map has a plugin associated with 
    it any attempted modifications of the plugin will be ignored.
*/

void QDeclarativeGraphicsGeoMap::setPlugin(QDeclarativeGeoServiceProvider *plugin)
{
    if (plugin_)
        return;

    plugin_ = plugin;

    emit pluginChanged(plugin_);

    serviceProvider_ = new QGeoServiceProvider(plugin_->name(),
                                               plugin_->parameterMap());

    // check for error

    mappingManager_ = serviceProvider_->mappingManager();

    // check for error

    mapData_ = mappingManager_->createMapData();
    mapData_->init();
    //mapData_->setParentItem(this);

    // setters
    mapData_->setWindowSize(size_);
    mapData_->setZoomLevel(zoomLevel_);
    mapData_->setCenter(center_->coordinate());
    mapData_->setMapType(QGraphicsGeoMap::MapType(mapType_));
    mapData_->setConnectivityMode(QGraphicsGeoMap::ConnectivityMode(connectivityMode_));

    for (int i = 0; i < objects_.size(); ++i)
        mapData_->addMapObject(objects_.at(i));
    objects_.clear();


    // setup signals

    connect(mapData_,
            SIGNAL(updateMapDisplay(QRectF)),
            this,
            SLOT(updateMapDisplay(QRectF)));

    connect(mapData_,
            SIGNAL(centerChanged(QGeoCoordinate)),
            this,
            SLOT(internalCenterChanged(QGeoCoordinate)));

    connect(mapData_,
            SIGNAL(mapTypeChanged(QGraphicsGeoMap::MapType)),
            this,
            SLOT(internalMapTypeChanged(QGraphicsGeoMap::MapType)));

    connect(mapData_,
            SIGNAL(connectivityModeChanged(QGraphicsGeoMap::ConnectivityMode)),
            this,
            SLOT(internalConnectivityModeChanged(QGraphicsGeoMap::ConnectivityMode)));

    connect(mapData_,
            SIGNAL(windowSizeChanged(QSizeF)),
            this,
            SIGNAL(sizeChanged(QSizeF)));

    connect(mapData_,
            SIGNAL(zoomLevelChanged(qreal)),
            this,
            SIGNAL(zoomLevelChanged(qreal)));
}

void QDeclarativeGraphicsGeoMap::updateMapDisplay(const QRectF &target)
{
    update(target);
}

QDeclarativeGeoServiceProvider* QDeclarativeGraphicsGeoMap::plugin() const
{
    return plugin_;
}

/*!
    \qmlproperty qreal Map::minimumZoomLevel

    This property holds the minimum valid zoom level for the map.
*/
qreal QDeclarativeGraphicsGeoMap::minimumZoomLevel() const
{
    if (mappingManager_)
        return mappingManager_->minimumZoomLevel();
    else
        return -1.0;
}

/*!
    \qmlproperty qreal Map::maximumZoomLevel

    This property holds the maximum valid zoom level for the map.
*/
qreal QDeclarativeGraphicsGeoMap::maximumZoomLevel() const
{
    if (mappingManager_)
        return mappingManager_->maximumZoomLevel();
    else
        return -1.0;
}

// TODO make these more QML like
//QList<MapType> QDeclarativeGraphicsGeoMap::supportedMapTypes() const;
//QList<ConnectivityMode> QDeclarativeGraphicsGeoMap::supportedConnectivityModes() const;

/*!
    \qmlproperty QSizeF Map::size

    This property holds the size of the map viewport.
*/
void QDeclarativeGraphicsGeoMap::setSize(const QSizeF &size)
{
    if (mapData_) {
        setWidth(size.width());
        setHeight(size.height());
        mapData_->setWindowSize(size);
    } else {
        if (size_ == size)
            return;

        size_ = size;

        emit sizeChanged(size_);
    }
        
}

QSizeF QDeclarativeGraphicsGeoMap::size() const
{
    if (mapData_)
        return mapData_->windowSize();
    else
        return size_;
}

/*!
    \qmlproperty qreal Map::zoomLevel

    This property holds the zoom level for the map.

    Larger values for the zoom level provide more detail.

    The default value is 8.0.
*/
void QDeclarativeGraphicsGeoMap::setZoomLevel(qreal zoomLevel)
{
    if (mapData_) {
        mapData_->setZoomLevel(zoomLevel);
    } else {
        if (zoomLevel_ == zoomLevel)
            return;

        zoomLevel_ = zoomLevel;

        emit zoomLevelChanged(zoomLevel_);
    }
}

qreal QDeclarativeGraphicsGeoMap::zoomLevel() const
{
    if (mapData_) {
        return mapData_->zoomLevel();
    } else {
        return zoomLevel_;
    }
}

/*!
    \qmlproperty Coordinate Map::center

    This property holds the coordinate which occupies the center of the 
    mapping viewport.

    The default value is an arbitrary valid coordinate.
*/
void QDeclarativeGraphicsGeoMap::setCenter(const QDeclarativeCoordinate *center)
{
    if (mapData_) {
        mapData_->setCenter(center->coordinate());
    } else {
        if (center_->coordinate() == center->coordinate())
            return;

        center_->setCoordinate(center->coordinate());

        emit declarativeCenterChanged(center_);
    }
}

QDeclarativeCoordinate* QDeclarativeGraphicsGeoMap::center()
{
    if (mapData_)
        center_->setCoordinate(mapData_->center());
    return center_;
}

void QDeclarativeGraphicsGeoMap::centerLatitudeChanged(double /*latitude*/)
{
    if (mapData_)
        mapData_->setCenter(center_->coordinate());
}

void QDeclarativeGraphicsGeoMap::centerLongitudeChanged(double /*longitude*/)
{
    if (mapData_)
        mapData_->setCenter(center_->coordinate());
}

void QDeclarativeGraphicsGeoMap::centerAltitudeChanged(double /*altitude*/)
{
    if (mapData_)
        mapData_->setCenter(center_->coordinate());
}

/*!
    \qmlproperty enumeration Map::mapType

    This property holds the type of map to display.

    The type can be one of:
    \list
    \o Map.StreetMap
    \o Map.SatelliteMapDay
    \o Map.SatelliteMapNight
    \o Map.TerrainMap
    \endlist

    The default value is determined by the plugin.
*/
void QDeclarativeGraphicsGeoMap::setMapType(QDeclarativeGraphicsGeoMap::MapType mapType)
{
    if (mapData_) {
        mapData_->setMapType(QGraphicsGeoMap::MapType(mapType));
    } else {
        if (mapType_ == mapType)
            return;

        mapType_ = mapType;

        emit mapTypeChanged(mapType_);
    }
}

QDeclarativeGraphicsGeoMap::MapType QDeclarativeGraphicsGeoMap::mapType() const
{
    if (mapData_) {
        return QDeclarativeGraphicsGeoMap::MapType(mapData_->mapType());
    } else {
        return mapType_;
    }
}

/*!
    \qmlproperty enumeration Map::connectivityMode

    This property holds the connectivity mode used to fetch the map data.

    The mode can be one of:
    \list
    \o Map.OfflineMode
    \o Map.OnlineMode
    \o Map.HybridMode
    \endlist

    The default value is determined by the plugin.
*/
void QDeclarativeGraphicsGeoMap::setConnectivityMode(QDeclarativeGraphicsGeoMap::ConnectivityMode connectivityMode)
{
    if (mapData_) {
        mapData_->setConnectivityMode(QGraphicsGeoMap::ConnectivityMode(connectivityMode));
    } else {
        if (connectivityMode_ == connectivityMode)
            return;

        connectivityMode_ = connectivityMode;

        emit connectivityModeChanged(connectivityMode_);
    }
}

QDeclarativeGraphicsGeoMap::ConnectivityMode QDeclarativeGraphicsGeoMap::connectivityMode() const
{
    if (mapData_)
        return QDeclarativeGraphicsGeoMap::ConnectivityMode(mapData_->connectivityMode());
    else
        return connectivityMode_;
}

/*!
    \qmlproperty list<QGeoMapObject> Map::objects
    \default

    This property holds the list of objects associated with this map.

    The various objects that can be added include:
    \list
    \o MapRectangle
    \o MapCircle
    \o MapText
    \o MapImage
    \o MapPolygon
    \o MapPolyline
    \o MapGroup
    \endlist
*/
QDeclarativeListProperty<QGeoMapObject> QDeclarativeGraphicsGeoMap::objects()
{
    return QDeclarativeListProperty<QGeoMapObject>(this,
            0,
            object_append,
            object_count,
            object_at,
            object_clear);
}

void QDeclarativeGraphicsGeoMap::object_append(QDeclarativeListProperty<QGeoMapObject> *prop, QGeoMapObject *mapObject)
{
    QDeclarativeGraphicsGeoMap *map = static_cast<QDeclarativeGraphicsGeoMap*>(prop->object);

    if (map->mapData_)
        map->mapData_->addMapObject(mapObject);
    else
        map->objects_.append(mapObject);
}

int QDeclarativeGraphicsGeoMap::object_count(QDeclarativeListProperty<QGeoMapObject> *prop)
{
    QDeclarativeGraphicsGeoMap *map = static_cast<QDeclarativeGraphicsGeoMap*>(prop->object);

    if (map->mapData_)
        return map->mapData_->mapObjects().size();
    else
        return map->objects_.size();
}

QGeoMapObject *QDeclarativeGraphicsGeoMap::object_at(QDeclarativeListProperty<QGeoMapObject> *prop, int index)
{
    QDeclarativeGraphicsGeoMap *map = static_cast<QDeclarativeGraphicsGeoMap*>(prop->object);

    if (map->mapData_)
        return map->mapData_->mapObjects().at(index);
    else
        return map->objects_.at(index);
}

void QDeclarativeGraphicsGeoMap::object_clear(QDeclarativeListProperty<QGeoMapObject> *prop)
{
    QDeclarativeGraphicsGeoMap *map = static_cast<QDeclarativeGraphicsGeoMap*>(prop->object);

    if (map->mapData_)
        map->mapData_->clearMapObjects();
    else
        map->objects_.clear();
}

/*!
    \qmlmethod Map::toCoordinate(QPointF screenPosition)

    Returns the coordinate which corresponds to the screen position 
    \a screenPosition.

    Returns an invalid coordinate if \a screenPosition is not within
    the current viewport.
*/

QDeclarativeCoordinate* QDeclarativeGraphicsGeoMap::toCoordinate(QPointF screenPosition) const
{
    QGeoCoordinate coordinate;

    if (mapData_)
        coordinate = mapData_->screenPositionToCoordinate(screenPosition);

    return new QDeclarativeCoordinate(coordinate,
                                      const_cast<QDeclarativeGraphicsGeoMap *>(this));
}

/*!
    \qmlmethod Map::toScreenPosition(Coordinate coordinate)

    Returns the screen position which corresponds to the coordinate 
    \a coordinate.

    Returns an invalid QPointF if \a coordinate is not within the 
    current viewport.
*/
QPointF QDeclarativeGraphicsGeoMap::toScreenPosition(QDeclarativeCoordinate* coordinate) const
{
    QPointF point;

    if (mapData_)
        point = mapData_->coordinateToScreenPosition(coordinate->coordinate());

    return point;
}

void QDeclarativeGraphicsGeoMap::pan(int dx, int dy)
{
    if (mapData_) {
        mapData_->pan(dx, dy);
        update();
    }
}

void QDeclarativeGraphicsGeoMap::internalCenterChanged(const QGeoCoordinate &coordinate)
{
    emit declarativeCenterChanged(new QDeclarativeCoordinate(coordinate, this));
}

void QDeclarativeGraphicsGeoMap::internalMapTypeChanged(QGraphicsGeoMap::MapType mapType)
{
    emit mapTypeChanged(QDeclarativeGraphicsGeoMap::MapType(mapType));
}

void QDeclarativeGraphicsGeoMap::internalConnectivityModeChanged(QGraphicsGeoMap::ConnectivityMode connectivityMode)
{
    emit connectivityModeChanged(QDeclarativeGraphicsGeoMap::ConnectivityMode(connectivityMode));
}

#include "moc_qdeclarativegraphicsgeomap_p.cpp"

QTM_END_NAMESPACE

