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

#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtGui/QIcon>
#include <QtCore/QDir>
#include <QtCore/QDebug>

#include "qgstreamerserviceplugin.h"

#ifdef QMEDIA_GSTREAMER_PLAYER
#include "qgstreamerplayerservice.h"
#endif

#if defined(QMEDIA_GSTREAMER_CAPTURE)
#include "qgstreamercaptureservice.h"
#endif

#ifdef QMEDIA_GSTREAMER_CAMERABIN
#include "camerabinservice.h"
#endif

#include <qmediaserviceprovider.h>

#include <linux/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/videodev2.h>


QStringList QGstreamerServicePlugin::keys() const
{
    return QStringList()
#ifdef QMEDIA_GSTREAMER_PLAYER
            << QLatin1String(Q_MEDIASERVICE_MEDIAPLAYER)
#endif

#ifdef QMEDIA_GSTREAMER_CAPTURE
            << QLatin1String(Q_MEDIASERVICE_AUDIOSOURCE)
            << QLatin1String(Q_MEDIASERVICE_CAMERA)
#elif defined(QMEDIA_GSTREAMER_CAMERABIN)
            << QLatin1String(Q_MEDIASERVICE_CAMERA)
#endif
    ;

}

QMediaService* QGstreamerServicePlugin::create(const QString &key)
{
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        gst_init(NULL, NULL);
    }

#ifdef QMEDIA_GSTREAMER_PLAYER
    if (key == QLatin1String(Q_MEDIASERVICE_MEDIAPLAYER))
        return new QGstreamerPlayerService;
#endif

#ifdef QMEDIA_GSTREAMER_CAMERABIN
    if (key == QLatin1String(Q_MEDIASERVICE_CAMERA) && CameraBinService::isCameraBinAvailable())
        return new CameraBinService(key);
#endif

#ifdef QMEDIA_GSTREAMER_CAPTURE
    if (key == QLatin1String(Q_MEDIASERVICE_AUDIOSOURCE))
        return new QGstreamerCaptureService(key);

    if (key == QLatin1String(Q_MEDIASERVICE_CAMERA))
        return new QGstreamerCaptureService(key);
#endif

    qWarning() << "Gstreamer service plugin: unsupported key:" << key;
    return 0;
}

void QGstreamerServicePlugin::release(QMediaService *service)
{
    delete service;
}

QList<QByteArray> QGstreamerServicePlugin::devices(const QByteArray &service) const
{
    if (service == Q_MEDIASERVICE_CAMERA) {
        if (m_cameraDevices.isEmpty())
            updateDevices();

        return m_cameraDevices;
    }

    return QList<QByteArray>();
}

QString QGstreamerServicePlugin::deviceDescription(const QByteArray &service, const QByteArray &device)
{
    if (service == Q_MEDIASERVICE_CAMERA) {
        if (m_cameraDevices.isEmpty())
            updateDevices();

        for (int i=0; i<m_cameraDevices.count(); i++)
            if (m_cameraDevices[i] == device)
                return m_cameraDescriptions[i];
    }

    return QString();
}

QVariant QGstreamerServicePlugin::deviceProperty(const QByteArray &service, const QByteArray &device, const QByteArray &property)
{
    Q_UNUSED(service);
    Q_UNUSED(device);
    Q_UNUSED(property);
    return QVariant();
}

void QGstreamerServicePlugin::updateDevices() const
{
    m_cameraDevices.clear();
    m_cameraDescriptions.clear();

#ifdef Q_WS_MAEMO_5
    m_cameraDevices << "/dev/video0" << "/dev/video1";
    m_cameraDescriptions << tr("Main Camera") << tr("Front Camera");
    return;
#endif

    QDir devDir("/dev");
    devDir.setFilter(QDir::System);

    QFileInfoList entries = devDir.entryInfoList(QStringList() << "video*");

    foreach( const QFileInfo &entryInfo, entries ) {
        //qDebug() << "Try" << entryInfo.filePath();

        int fd = ::open(entryInfo.filePath().toLatin1().constData(), O_RDWR );
        if (fd == -1)
            continue;

        bool isCamera = false;

        v4l2_input input;
        memset(&input, 0, sizeof(input));
        for (; ::ioctl(fd, VIDIOC_ENUMINPUT, &input) >= 0; ++input.index) {
            if(input.type == V4L2_INPUT_TYPE_CAMERA || input.type == 0) {
                isCamera = ::ioctl(fd, VIDIOC_S_INPUT, input.index) != 0;
                break;
            }
        }

        if (isCamera) {
            // find out its driver "name"
            QString name;
            struct v4l2_capability vcap;
            memset(&vcap, 0, sizeof(struct v4l2_capability));

            if (ioctl(fd, VIDIOC_QUERYCAP, &vcap) != 0)
                name = entryInfo.fileName();
            else
                name = QString((const char*)vcap.card);
            //qDebug() << "found camera: " << name;

            m_cameraDevices.append(entryInfo.filePath().toLocal8Bit());
            m_cameraDescriptions.append(name);
        }
        ::close(fd);
    }
}

Q_EXPORT_PLUGIN2(qtmedia_gstengine, QGstreamerServicePlugin);
