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

#ifndef S60CAMERASETTINGS_H
#define S60CAMERASETTINGS_H

#include "qcamera.h"

#include "s60cameraengine.h"
#include "s60cameraengineobserver.h"

#include <e32base.h>

QT_USE_NAMESPACE

/*
 * Class handling CCamera AdvancedSettings and ImageProcessing operations.
 */
class S60CameraSettings : public QObject,
                          public MAdvancedSettingsObserver
{
    Q_OBJECT

public: // Static Contructor & Destructor

    static S60CameraSettings* New(int &error, QObject *parent = 0, CCameraEngine *engine = 0);
    ~S60CameraSettings();

public: // Methods

    // Focus
    QCameraFocus::FocusMode focusMode();
    void setFocusMode(QCameraFocus::FocusMode mode);
    QCameraFocus::FocusModes supportedFocusModes();
    void cancelFocusing();

    // Zoom
    qreal opticalZoomFactorL() const;
    void setOpticalZoomFactorL(const qreal zoomFactor);
    QList<qreal> *supportedDigitalZoomFactors();
    qreal digitalZoomFactorL() const;
    void setDigitalZoomFactorL(const qreal zoomFactor);

    // Flash
    bool isFlashReady();

    // Exposure
    void setExposureMode(QCameraExposure::ExposureMode mode);
    void lockExposure(bool lock);
    bool isExposureLocked();

    // Metering Mode
    QCameraExposure::MeteringMode meteringMode();
    void setMeteringMode(QCameraExposure::MeteringMode mode);
    bool isMeteringModeSupported(QCameraExposure::MeteringMode mode);

    // ISO Sensitivity
    int isoSensitivity();
    void setManualIsoSensitivity(int iso);
    void setAutoIsoSensitivity();
    QList<int> supportedIsoSensitivities();

    // Aperture
    qreal aperture();
    void setManualAperture(qreal aperture);
    QList<qreal> supportedApertures();

    // Shutter Speed
    TInt shutterSpeed();
    void setManualShutterSpeed(TInt speed);
    QList<qreal> supportedShutterSpeeds();

    // ExposureCompensation
    qreal exposureCompensation();
    void setExposureCompensation(qreal ev);
    QList<qreal> supportedExposureCompensationValues();

    // Sharpening Level
    int sharpeningLevel() const;
    void setSharpeningLevel(int value);
    bool isSharpeningSupported() const;

    // Saturation
    int saturation();
    void setSaturation(int value);

Q_SIGNALS: // Notifications

    // For QCameraExposureControl
    void flashReady(bool ready);
    void apertureChanged();
    void apertureRangeChanged();
    void shutterSpeedChanged();
    void isoSensitivityChanged();

    // For QCameraLocksControl
    void exposureStatusChanged(QCamera::LockStatus, QCamera::LockChangeReason);
    void focusStatusChanged(QCamera::LockStatus, QCamera::LockChangeReason);

    // Errors
    void error(int, const QString&);

protected: // Protected constructors

    S60CameraSettings(QObject *parent, CCameraEngine *engine);
    void ConstructL();

protected: // MAdvancedSettingsObserver

    void HandleAdvancedEvent(const TECAMEvent& aEvent);

private: // Internal

    bool queryAdvancedSettingsInfo();

private: // Enums

    enum EcamErrors {
        KErrECamCameraDisabled =        -12100, // The camera has been disabled, hence calls do not succeed
        KErrECamSettingDisabled =       -12101, // This parameter or operation is supported, but presently is disabled.
        KErrECamParameterNotInRange =   -12102, // This value is out of range.
        KErrECamSettingNotSupported =   -12103, // This parameter or operation is not supported.
        KErrECamNotOptimalFocus =       -12104  // The optimum focus is lost
    };

private: // Data

#ifndef S60_31_PLATFORM // Post S60 3.1 Platforms
    CCamera::CCameraAdvancedSettings    *m_advancedSettings;
    CCamera::CCameraImageProcessing     *m_imageProcessingSettings;
#endif // S60_31_PLATFORM
    CCameraEngine                       *m_cameraEngine;
    QList<qreal>                        m_supportedDigitalZoomFactors;
};

#endif // S60CAMERASETTINGS_H
