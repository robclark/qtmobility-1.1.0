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

#include "qgeomappingmanager.h"
#include "qgeomappingmanager_p.h"
#include "qgeomappingmanagerengine.h"

#include <QNetworkProxy>
#include <QLocale>

QTM_BEGIN_NAMESPACE

/*!
    \class QGeoMappingManager
    \brief The QGeoMappingManager class provides support for displaying
    and interacting with maps.

    \inmodule QtLocation

    \ingroup maps-mapping

    A QGeoMappingManager instance can create QGeoMapData instances with
    createMapData(). The QGeoMapData instances can be used to contain and
    manage information concerning what a particular QGraphicsGeoMap is viewing.

    The functions in this class will typically not be used by clients of this
    API, as the most common uses will only need to obtain a QGeoMappingManager
    instance and associate it with a QGraphicsGeoMap instance:
    \code
        QGeoServiceProvider serviceProvider("nokia");
        QGeoMappingManager *manager = serviceProvider.mappingManager();
        QGraphicsGeoMap *geoMap = new QGraphicsGeoMap(manager);
    \endcode

    This could have been simplified by having the plugin return a
    QGraphicsGeoMap instance instead, but this approach allows users to
    subclass QGraphicsGeoMap in order to override the standard event handlers
    and implement custom map behaviours.
*/

/*!
    Constructs a new manager with the specified \a parent and with the
    implementation provided by \a engine.

    This constructor is used internally by QGeoServiceProviderFactory. Regular
    users should aquire instance of QGeoMappingManager with
    QGeoServiceProvider::mappingManager();
*/
QGeoMappingManager::QGeoMappingManager(QGeoMappingManagerEngine *engine, QObject *parent)
    : QObject(parent),
      d_ptr(new QGeoMappingManagerPrivate)
{
    d_ptr->engine = engine;
    if (d_ptr->engine) {
        d_ptr->engine->setParent(this);
    } else {
        qFatal("The mapping manager engine that was set for this mapping manager was NULL.");
    }
}

/*!
    Destroys this mapping manager.
*/
QGeoMappingManager::~QGeoMappingManager()
{
    delete d_ptr;
}

/*!
    Returns the name of the engine which implements the behaviour of this
    mapping manager.

    The combination of managerName() and managerVersion() should be unique
    amongst the plugin implementations.
*/
QString QGeoMappingManager::managerName() const
{
    return d_ptr->engine->managerName();
}

/*!
    Returns the version of the engine which implements the behaviour of this
    mapping manager.

    The combination of managerName() and managerVersion() should be unique
    amongst the plugin implementations.
*/
int QGeoMappingManager::managerVersion() const
{
    return d_ptr->engine->managerVersion();
}

/*!
    Returns a new QGeoMapData instance which will be managed by this manager.
*/
QGeoMapData* QGeoMappingManager::createMapData()
{
    return d_ptr->engine->createMapData();
}

/*!
    Returns a list of the map types supported by this manager.
*/
QList<QGraphicsGeoMap::MapType> QGeoMappingManager::supportedMapTypes() const
{
    return d_ptr->engine->supportedMapTypes();
}

/*!
    Returns a list of the connectivity modes supported by this manager.
*/
QList<QGraphicsGeoMap::ConnectivityMode> QGeoMappingManager::supportedConnectivityModes() const
{
    return d_ptr->engine->supportedConnectivityModes();
}

/*!
    Returns the minimum zoom level supported by this manager.

    Larger values of the zoom level correspond to more detailed views of the
    map.
*/
qreal QGeoMappingManager::minimumZoomLevel() const
{
    return d_ptr->engine->minimumZoomLevel();
}

/*!
    Returns the maximum zoom level supported by this manager.

    Larger values of the zoom level correspond to more detailed views of the
    map.
*/
qreal QGeoMappingManager::maximumZoomLevel() const
{
    return d_ptr->engine->maximumZoomLevel();
}

/*!
    Sets the locale to be used by the this manager to \a locale.

    If this mapping manager supports returning map labels
    in different languages, they will be returned in the language of \a locale.

    The locale used defaults to the system locale if this is not set.
*/
void QGeoMappingManager::setLocale(const QLocale &locale)
{
    d_ptr->engine->setLocale(locale);
}

/*!
    Returns the locale used to hint to this mapping manager about what
    language to use for map labels.
*/
QLocale QGeoMappingManager::locale() const
{
    return d_ptr->engine->locale();
}

/*******************************************************************************
*******************************************************************************/

QGeoMappingManagerPrivate::QGeoMappingManagerPrivate()
    : engine(0) {}

QGeoMappingManagerPrivate::~QGeoMappingManagerPrivate()
{
    if (engine)
        delete engine;
}

#include "moc_qgeomappingmanager.cpp"

QTM_END_NAMESPACE
