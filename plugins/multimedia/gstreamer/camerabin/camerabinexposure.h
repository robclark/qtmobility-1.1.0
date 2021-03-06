/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CAMERABINEXPOSURECONTROL_MAEMO_H
#define CAMERABINEXPOSURECONTROL_MAEMO_H

#include <qcamera.h>
#include <qcameraexposurecontrol.h>

#include <gst/gst.h>
#include <glib.h>

class CameraBinSession;

QT_USE_NAMESPACE

class Q_MULTIMEDIA_EXPORT CameraBinExposure : public QCameraExposureControl
{
    Q_OBJECT

public:
    CameraBinExposure(CameraBinSession *session);
    virtual ~CameraBinExposure();

    QCameraExposure::ExposureMode exposureMode() const;
    void setExposureMode(QCameraExposure::ExposureMode mode);
    bool isExposureModeSupported(QCameraExposure::ExposureMode mode) const;

    QCameraExposure::MeteringMode meteringMode() const;
    void setMeteringMode(QCameraExposure::MeteringMode mode);
    bool isMeteringModeSupported(QCameraExposure::MeteringMode mode) const;

    bool isParameterSupported(ExposureParameter parameter) const;
    QVariant exposureParameter(ExposureParameter parameter) const;
    ParameterFlags exposureParameterFlags(ExposureParameter parameter) const;
    QVariantList supportedParameterRange(ExposureParameter parameter) const;
    bool setExposureParameter(ExposureParameter parameter, const QVariant& value);

    QString extendedParameterName(ExposureParameter parameter);

private:
    CameraBinSession *m_session;    
};

#endif // CAMERABINEXPOSURECONTROL_MAEMO_H
