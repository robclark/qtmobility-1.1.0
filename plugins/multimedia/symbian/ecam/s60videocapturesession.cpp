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

#include <QtCore/qstring.h>
#include <QtCore/qdir.h>

#include "s60videocapturesession.h"
#include "s60cameraconstants.h"

#include <utf.h>
#include <bautils.h>

#ifdef S60_DEVVIDEO_RECORDING_SUPPORTED
#include <mmf/devvideo/devvideorecord.h>
#endif

S60VideoCaptureSession::S60VideoCaptureSession(QObject *parent) :
    QObject(parent),
    m_cameraEngine(NULL),
    m_videoRecorder(NULL),
    m_error(KErrNone),
    m_cameraStarted(false),
    m_captureState(ENotInitialized),    // Default state
    m_sink(QUrl()),
    m_requestedSink(QUrl()),
    m_videoQuality(QtMultimediaKit::VeryHighQuality * KSymbianImageQualityCoefficient),   // Default video quality
    m_container(QString()),
    m_requestedContainer(QString()),
    m_muted(false),
    m_maxClipSize(-1),
    m_videoControllerMap(QHash<int, QHash<int,VideoFormatData> >()),
    m_videoParametersForEncoder(QList<MaxResolutionRatesAndTypes>()),
    m_openWhenReady(false),
    m_prepareAfterOpenComplete(false),
    m_startAfterPrepareComplete(false),
    m_uncommittedSettings(false),
    m_commitSettingsWhenReady(false)
{
#ifdef S60_DEVVIDEO_RECORDING_SUPPORTED
    // Populate info of supported codecs, and their resolution, etc.
    TRAPD(err, doPopulateVideoCodecsDataL());
    setError(err, QString("Failed to gather video codec information."));
#endif // S60_DEVVIDEO_RECORDING_SUPPORTED
}

S60VideoCaptureSession::~S60VideoCaptureSession()
{
    if (m_captureState >= ERecording) {
        m_videoRecorder->Stop();
    }

    if (m_captureState >= EInitialized) {
        m_videoRecorder->Close();
    }

    if (m_videoRecorder) {
        delete m_videoRecorder;
        m_videoRecorder = NULL;
    }

    // Clear all data structures
    foreach (MaxResolutionRatesAndTypes structure, m_videoParametersForEncoder) {
        structure.frameRatePictureSizePair.clear();
        structure.mimeTypes.clear();
    }
    m_videoParametersForEncoder.clear();

    m_videoCodeclist.clear();
    m_audioCodeclist.clear();

    QList<TInt> controllers = m_videoControllerMap.keys();
    for (int i = 0; i < controllers.size(); ++i) {
        foreach(VideoFormatData data, m_videoControllerMap[controllers[i]]){
            data.supportedMimeTypes.clear();
        }
        m_videoControllerMap[controllers[i]].clear();
    }
    m_videoControllerMap.clear();
}

/*
 * This function can be used both internally and from Control classes using this session.
 * The error notification will go to client application through QMediaRecorder error signal.
 */
void S60VideoCaptureSession::setError(const TInt error, const QString &description)
{
    if (error == KErrNone)
        return;

    m_error = error;
    QMediaRecorder::Error recError = fromSymbianErrorToQtMultimediaError(m_error);

    // Stop/Close/Reset only of other than "not supported" error
    if (m_error != KErrNotSupported) {
        if (m_captureState >= ERecording)
            m_videoRecorder->Stop();

        if (m_captureState >= EInitialized) {
            m_videoRecorder->Close();
        }

        // Reset state
        if (m_captureState != ENotInitialized) {
            m_captureState = ENotInitialized;
            emit stateChanged(m_captureState);
        }
    }

    emit this->error(recError, description);

    // Reset only of other than "not supported" error
    if (m_error != KErrNotSupported)
        resetSession();
    else
        m_error = KErrNone; // Reset error
}

QMediaRecorder::Error S60VideoCaptureSession::fromSymbianErrorToQtMultimediaError(int aError)
{
    switch(aError) {
        case KErrNone:
            return QMediaRecorder::NoError; // No errors have occurred
        case KErrArgument:
        case KErrNotSupported:
            return QMediaRecorder::FormatError; // The feature/format is not supported
        case KErrNoMemory:
        case KErrNotFound:
        case KErrBadHandle:
            return QMediaRecorder::ResourceError; // Not able to use camera/recorder resources

        default:
            return QMediaRecorder::ResourceError; // Other error has occured
    }
}

/*
 * This function applies all recording settings to make latency during the
 * start of the recording as short as possible. After this it is not possible to
 * set settings (inc. output location) before stopping the recording.
 */
void S60VideoCaptureSession::applyAllSettings()
{
    switch (m_captureState) {
        case ENotInitialized:
            setError(KErrNotReady, QString("Cannot apply settings before camera is started."));
            return;
        case EInitializing:
            m_commitSettingsWhenReady = true;
            return;
        case EInitialized:
            setOutputLocation(QUrl());
            m_prepareAfterOpenComplete = true;
            return;
        case EOpening:
            m_prepareAfterOpenComplete = true;
            return;
        case EOpenComplete:
            // Do nothing, ready to commit
            break;
        case EPreparing:
            m_commitSettingsWhenReady = true;
            return;
        case EPrepared:
            // Revert state internally, since logically applying settings means going
            // from OpenComplete ==> Preparing ==> Prepared.
            m_captureState = EOpenComplete;
            break;
        case ERecording:
        case EPaused:
            setError(KErrNotReady, QString("Cannot apply settings while recording."));
            return;

        default:
            setError(KErrGeneral, QString("Unexpected camera error."));
            return;
    }

    // Commit settings - State is now either OpenComplete or Prepared
    commitVideoEncoderSettings();

    // If capture state has been reset, a different container was requested
    if (m_captureState == EOpening) {
        return;
    }

    // Start preparing
    m_captureState = EPreparing;
    emit stateChanged(m_captureState);

    if (m_cameraEngine->IsCameraReady()) {
        m_videoRecorder->Prepare();
    }
}

void S60VideoCaptureSession::setCameraHandle(CCameraEngine* cameraHandle)
{
    m_cameraEngine = cameraHandle;

    resetSession();
}

void S60VideoCaptureSession::doInitializeVideoRecorderL()
{
    if (m_captureState > ENotInitialized) {
        resetSession();
    }

    m_captureState = EInitializing;
    emit stateChanged(m_captureState);

    // Open Dummy file to be able to query supported settings
    int cameraHandle = m_cameraEngine->Camera()->Handle();

    TUid controllerUid;
    TUid formatUid;
    selectController(m_requestedContainer, controllerUid, formatUid);

    if (m_videoRecorder) {
        // File open completes in MvruoOpenComplete
        TRAPD(err, m_videoRecorder->OpenFileL(KDummyVideoFile, cameraHandle, controllerUid, formatUid));
        setError(err, QString("Failed to initialize video recorder."));
        m_container = m_requestedContainer;
    } else {
        setError(KErrNotReady, QString("Unexpected camera error."));
    }
}

void S60VideoCaptureSession::resetSession()
{
    if (m_videoRecorder) {
        delete m_videoRecorder;
        m_videoRecorder = NULL;
    }

    if (m_captureState != ENotInitialized) {
        m_captureState = ENotInitialized;
        emit stateChanged(m_captureState);
    }

    // Reset error to be able to recover
    m_error = KErrNone;

    // Reset flags
    m_openWhenReady = false;
    m_prepareAfterOpenComplete = false;
    m_startAfterPrepareComplete = false;
    m_uncommittedSettings = false;
    m_commitSettingsWhenReady = false;

    TRAPD(err, m_videoRecorder = CVideoRecorderUtility::NewL(*this));
    setError(err, QString("Failure in creation of video recorder device."));

    updateVideoCaptureContainers();
}

QList<QSize> S60VideoCaptureSession::supportedVideoResolutions(bool *continuous)
{
    QList<QSize> list;

    if (m_videoParametersForEncoder.count() > 0) {

        // Also arbitrary resolutions are supported
        if (continuous)
            *continuous = true;

        // Append all supported resolutions to the list
        foreach (MaxResolutionRatesAndTypes parameters, m_videoParametersForEncoder)
            for (int i = 0; i < parameters.frameRatePictureSizePair.count(); ++i)
                if (!list.contains(parameters.frameRatePictureSizePair[i].frameSize))
                    list.append(parameters.frameRatePictureSizePair[i].frameSize);
    }

#ifdef Q_CC_NOKIAX86 // Emulator
    list << QSize(160, 120);
    list << QSize(352, 288);
    list << QSize(640,480);
#endif

    return list;
}

QList<QSize> S60VideoCaptureSession::supportedVideoResolutions(const QVideoEncoderSettings &settings, bool *continuous)
{
    QList<QSize> supportedFrameSizes;

    if (settings.codec().isEmpty())
        return supportedFrameSizes;

    if (!m_videoCodeclist.contains(settings.codec(), Qt::CaseInsensitive)) {
        return supportedFrameSizes;
    }

    // Also arbitrary resolutions are supported
    if (continuous)
        *continuous = true;

    // Find maximum resolution (using defined framerate if set)
    for (int i = 0; i < m_videoParametersForEncoder.count(); ++i) {
        // Check if encoder supports the requested codec
        if (!m_videoParametersForEncoder[i].mimeTypes.contains(settings.codec(), Qt::CaseInsensitive)) {
            continue;
        }

        foreach (SupportedFrameRatePictureSize pair, m_videoParametersForEncoder[i].frameRatePictureSizePair) {
            if (!supportedFrameSizes.contains(pair.frameSize)) {
                QSize maxForMime = maximumResolutionForMimeType(settings.codec());
                if (settings.frameRate() != 0) {
                    if(settings.frameRate() <= pair.frameRate) {
                        if ((pair.frameSize.width() * pair.frameSize.height()) <= (maxForMime.width() * maxForMime.height()))
                            supportedFrameSizes.append(pair.frameSize);
                    }
                } else {
                    if ((pair.frameSize.width() * pair.frameSize.height()) <= (maxForMime.width() * maxForMime.height()))
                        supportedFrameSizes.append(pair.frameSize);
                }
            }
        }
    }

#ifdef Q_CC_NOKIAX86 // Emulator
    supportedFrameSizes << QSize(160, 120);
    supportedFrameSizes << QSize(352, 288);
    supportedFrameSizes << QSize(640,480);
#endif

    return supportedFrameSizes;
}

QList<qreal> S60VideoCaptureSession::supportedVideoFrameRates(bool *continuous)
{
    QList<qreal> list;

    if (m_videoParametersForEncoder.count() > 0) {
        // Insert min and max to the list
        list.append(1.0); // Use 1fps as sensible minimum
        qreal foundMaxFrameRate(0.0);

        // Also arbitrary framerates are supported
        if (continuous)
            *continuous = true;

        // Find max framerate
        foreach (MaxResolutionRatesAndTypes parameters, m_videoParametersForEncoder) {
            for (int i = 0; i < parameters.frameRatePictureSizePair.count(); ++i) {
                qreal maxFrameRate = parameters.frameRatePictureSizePair[i].frameRate;
                if (maxFrameRate > foundMaxFrameRate)
                    foundMaxFrameRate = maxFrameRate;
            }
        }

        list.append(foundMaxFrameRate);
    }

    // Add also other standard framerates to the list
    if (!list.isEmpty()) {
        if (list.last() > 30)
            list.insert(1, 30);
        if (list.last() > 20)
            list.insert(1, 20);
        if (list.last() > 15)
            list.insert(1, 15);
        if (list.last() > 10)
            list.insert(1, 10);
    }

#ifdef Q_CC_NOKIAX86 // Emulator
    list << 30.0 << 25.0 << 15.0 << 10.0 << 5.0;
#endif

    return list;
}

QList<qreal> S60VideoCaptureSession::supportedVideoFrameRates(const QVideoEncoderSettings &settings, bool *continuous)
{
    QList<qreal> supportedFrameRates;

    if (settings.codec().isEmpty())
        return supportedFrameRates;
    if (!m_videoCodeclist.contains(settings.codec(), Qt::CaseInsensitive))
        return supportedFrameRates;

    // Also arbitrary framerates are supported
    if (continuous)
        *continuous = true;

    // Find maximum framerate (using defined resolution if set)
    for (int i = 0; i < m_videoParametersForEncoder.count(); ++i) {
        // Check if encoder supports the requested codec
        if (!m_videoParametersForEncoder[i].mimeTypes.contains(settings.codec(), Qt::CaseInsensitive))
            continue;

        foreach (SupportedFrameRatePictureSize pair, m_videoParametersForEncoder[i].frameRatePictureSizePair) {
            if (!supportedFrameRates.contains(pair.frameRate)) {
                qreal maxRateForMime = maximumFrameRateForMimeType(settings.codec());
                if (settings.resolution().width() != 0 && settings.resolution().height() != 0) {
                    if((settings.resolution().width() * settings.resolution().height()) <= (pair.frameSize.width() * pair.frameSize.height())) {
                        if (pair.frameRate <= maxRateForMime)
                            supportedFrameRates.append(pair.frameRate);
                    }
                } else {
                    if (pair.frameRate <= maxRateForMime)
                        supportedFrameRates.append(pair.frameRate);
                }
            }
        }
    }

    // Add also other standard framerates to the list if max is higher
    if (!supportedFrameRates.isEmpty()) {
        if (supportedFrameRates.last() > 30)
            supportedFrameRates.insert(1, 30);
        if (supportedFrameRates.last() > 25)
            supportedFrameRates.insert(1, 25);
        if (supportedFrameRates.last() > 15)
            supportedFrameRates.insert(1, 15);
        if (supportedFrameRates.last() > 10)
            supportedFrameRates.insert(1, 10);
    }

#ifdef Q_CC_NOKIAX86 // Emulator
    supportedFrameRates << 30.0 << 25.0 << 15.0 << 10.0 << 5.0;
#endif
    return supportedFrameRates;
}

bool S60VideoCaptureSession::setOutputLocation(const QUrl &sink)
{
    m_requestedSink = sink;

    if (m_error)
        return false;

    switch (m_captureState) {
        case ENotInitialized:
        case EInitializing:
        case EOpening:
        case EPreparing:
            m_openWhenReady = true;
            return true;

        case EInitialized:
        case EOpenComplete:
        case EPrepared:
            // Continue
            break;

        case ERecording:
        case EPaused:
            setError(KErrNotReady, QString("Cannot set file name while recording."));
            return false;

        default:
            setError(KErrGeneral, QString("Unexpected camera error."));
            return false;
    }

    // Empty URL - Use default file name and path (C:\Data\Videos\video.mp4)
    if (sink.isEmpty()) {
        // Make sure default directory exists
        QDir videoDir(QDir::rootPath());
        if (!videoDir.exists(KDefaultVideoPath)) {
            videoDir.mkpath(KDefaultVideoPath);
        }
        QString defaultFile = KDefaultVideoPath;
        defaultFile.append("\\");
        defaultFile.append(KDefaultVideoFileName);
        m_sink.setUrl(defaultFile);

    } else { // Non-empty URL

        QString fullUrl = sink.scheme();

        // Relative URL
        if (sink.isRelative()) {

            // Extract file name and path from the URL
            fullUrl = KDefaultVideoPath;
            fullUrl.append("\\");
            fullUrl.append(QDir::toNativeSeparators(sink.path()));


        // Absolute URL
        } else {
            // Extract file name and path from the URL
            if (fullUrl == "file") {
                fullUrl = QDir::toNativeSeparators(sink.path().right(sink.path().length() - 1));
            } else {
                fullUrl.append(":");
                fullUrl.append(QDir::toNativeSeparators(sink.path()));
            }
        }

        QString fileName = fullUrl.right(fullUrl.length() - fullUrl.lastIndexOf("\\") - 1);
        QString directory = fullUrl.left(fullUrl.lastIndexOf("\\"));
        if (directory.lastIndexOf("\\") == (directory.length() - 1))
            directory = directory.left(directory.length() - 1);

        // URL is Absolute path, not including file name
        if (!fileName.contains(".")) {
            if (fileName != "") {
                directory.append("\\");
                directory.append(fileName);
            }
            fileName = KDefaultVideoFileName;
        }

        // Make sure absolute directory exists
        QDir videoDir(QDir::rootPath());
        if (!videoDir.exists(directory)) {
            videoDir.mkpath(directory);
        }

        QString resolvedURL = directory;
        resolvedURL.append("\\");
        resolvedURL.append(fileName);
        m_sink = QUrl(resolvedURL);
    }


    // State is either Initialized, OpenComplete or Prepared, Close previously opened file
    if (m_videoRecorder) {
        m_videoRecorder->Close();
    }
    else
        setError(KErrNotReady, QString("Unexpected camera error."));

    // Open file
    QString fileName = QDir::toNativeSeparators(m_sink.toString());
    TPtrC16 fileSink(reinterpret_cast<const TUint16*>(fileName.utf16()));

    int cameraHandle = m_cameraEngine->Camera()->Handle();

    TUid controllerUid;
    TUid formatUid;
    selectController(m_requestedContainer, controllerUid, formatUid);

    if (m_videoRecorder) {
        // File open completes in MvruoOpenComplete
        TRAPD(err, m_videoRecorder->OpenFileL(fileSink, cameraHandle, controllerUid, formatUid));
        setError(err, QString("Failed to initialize video recorder."));
        m_container = m_requestedContainer;
        m_captureState = EOpening;
        emit stateChanged(m_captureState);
    }
    else
        setError(KErrNotReady, QString("Unexpected camera error."));

    m_uncommittedSettings = true;
    return true;
}

QUrl S60VideoCaptureSession::outputLocation() const
{
    return m_sink;
}

qint64 S60VideoCaptureSession::position()
{
    qint64 position = 0;
    // Update position only if recording is ongoing
    if ((m_captureState == ERecording) && m_videoRecorder) {
        TRAPD(err, position = m_videoRecorder->DurationL().Int64() / 1000);
        setError(err, QString("Cannot retrieve video position."));
    }

    return position;
}

S60VideoCaptureSession::TVideoCaptureState S60VideoCaptureSession::state() const
{
    return m_captureState;
}

bool S60VideoCaptureSession::isMuted() const
{
    return m_muted;
}

void S60VideoCaptureSession::setMuted(const bool muted)
{
    // CVideoRecorderUtility can mute/unmute only if not recording
    if (m_captureState > EPrepared)
        return;

    // Check if request is already active
    if (muted == isMuted())
        return;

    m_muted = muted;

    m_uncommittedSettings = true;
}

void S60VideoCaptureSession::commitVideoEncoderSettings()
{
    if (m_captureState == EOpenComplete) {
        if (m_container != m_requestedContainer) {
            setOutputLocation(m_requestedSink);
            return;
        }

        TRAPD(err, doSetCodecsL(m_audioSettings.codec(), m_videoSettings.codec()));
        setError(err, QString("Failed to set audio/video codec."));
        doSetVideoResolution(m_videoSettings.resolution());
        doSetFrameRate(m_videoSettings.frameRate());
        doSetBitrate(m_videoSettings.bitRate());

#ifndef S60_3X_PLATFORM
        TRAP(err, m_videoRecorder->SetVideoQualityL(m_videoQuality));
        if (err != KErrNotSupported) {
            setError(err, QString("Setting video quality failed."));
        }
#endif // S60_3X_PLATFORM

        // Audio/Video EncodingMode are not supported in Symbian

        TRAP(err, m_videoRecorder->SetAudioBitRateL((TInt)m_audioSettings.bitRate()));
        if (err != KErrNotSupported)
            setError(err, QString("Setting audio bitrate failed."));

#ifndef S60_31_PLATFORM
        if (m_audioSettings.sampleRate() != -1) {
            TRAP(err, m_videoRecorder->SetAudioSampleRateL((TInt)m_audioSettings.sampleRate()));
            if (err != KErrNotSupported)
                setError(err, QString("Setting audio sample rate failed."));
        }

        TRAP(err, m_videoRecorder->SetAudioChannelsL((TUint)m_audioSettings.channelCount()));
        if (err != KErrNotSupported)
            setError(err, QString("Setting audio channel count failed."));
#endif // S60_31_PLATFORM

        // Note difference between muted and isEnabled (true for the other means false for the other)
        TBool isAudioEnabled = EFalse;
        TRAP(err, isAudioEnabled = m_videoRecorder->AudioEnabledL());
        setError(err, QString("Failure when checking if audio is enabled."));
        if (m_muted == (bool)isAudioEnabled) {
            TRAP(err, m_videoRecorder->SetAudioEnabledL((TBool)!m_muted));
            if (err) {
                if (err != KErrNotSupported)
                    setError(err, QString("Failed to mute/unmute audio."));
            }
            else
                emit mutedChanged(m_muted);
        }

        m_uncommittedSettings = false; // Reset
    }
}

void S60VideoCaptureSession::videoEncoderSettings(QVideoEncoderSettings &videoSettings) const
{
    videoSettings = m_videoSettings;
}

void S60VideoCaptureSession::audioEncoderSettings(QAudioEncoderSettings &audioSettings) const
{
    audioSettings = m_audioSettings;
}

void S60VideoCaptureSession::setVideoCaptureQuality(const QtMultimediaKit::EncodingQuality quality,
                                                    const VideoQualityDefinition mode)
{
#ifndef S60_3X_PLATFORM // S60 5.0 or later
    Q_UNUSED(mode);
    m_videoQuality = quality * KSymbianImageQualityCoefficient;
    if (m_videoQuality == 0)
        m_videoQuality = 25;
#else // S60 3.1 or 3.2
    // Sensible presets
    switch (mode) {
        case ENoVideoQuality:
            // Do nothing
            break;
        case EOnlyVideoQuality:
            if (quality == QtMultimediaKit::VeryLowQuality) {
                m_videoSettings.setResolution(QSize(128,96));
                m_videoSettings.setFrameRate(10);
                m_videoSettings.setBitRate(64000);
            } else if (quality == QtMultimediaKit::LowQuality) {
                m_videoSettings.setResolution(QSize(176,144));
                m_videoSettings.setFrameRate(15);
                m_videoSettings.setBitRate(64000);
            } else if (quality == QtMultimediaKit::NormalQuality) {
                m_videoSettings.setResolution(QSize(160,120));
                m_videoSettings.setFrameRate(15);
                m_videoSettings.setBitRate(128000);
            } else if (quality == QtMultimediaKit::HighQuality) {
                m_videoSettings.setResolution(QSize(352,288));
                m_videoSettings.setFrameRate(15);
                m_videoSettings.setBitRate(384000);
            } else if (quality == QtMultimediaKit::VeryHighQuality) {
                m_videoSettings.setResolution(QSize(640,480));
                m_videoSettings.setFrameRate(15);
                m_videoSettings.setBitRate(2000000);
            }
            break;
        case EVideoQualityAndResolution:
            if (quality == QtMultimediaKit::VeryLowQuality) {
                m_videoSettings.setFrameRate(10);
                m_videoSettings.setBitRate(64000);
            } else if (quality == QtMultimediaKit::LowQuality) {
                m_videoSettings.setFrameRate(15);
                m_videoSettings.setBitRate(64000);
            } else if (quality == QtMultimediaKit::NormalQuality) {
                m_videoSettings.setFrameRate(15);
                m_videoSettings.setBitRate(128000);
            } else if (quality == QtMultimediaKit::HighQuality) {
                m_videoSettings.setFrameRate(15);
                m_videoSettings.setBitRate(384000);
            } else if (quality == QtMultimediaKit::VeryHighQuality) {
                m_videoSettings.setFrameRate(15);
                m_videoSettings.setBitRate(2000000);
            }
            break;
        case EVideoQualityAndFrameRate:
            if (quality == QtMultimediaKit::VeryLowQuality) {
                m_videoSettings.setResolution(QSize(128,96));
                m_videoSettings.setBitRate(64000);
            } else if (quality == QtMultimediaKit::LowQuality) {
                m_videoSettings.setResolution(QSize(176,144));
                m_videoSettings.setBitRate(64000);
            } else if (quality == QtMultimediaKit::NormalQuality) {
                m_videoSettings.setResolution(QSize(160,120));
                m_videoSettings.setBitRate(128000);
            } else if (quality == QtMultimediaKit::HighQuality) {
                m_videoSettings.setResolution(QSize(352,288));
                m_videoSettings.setBitRate(384000);
            } else if (quality == QtMultimediaKit::VeryHighQuality) {
                m_videoSettings.setResolution(QSize(640,480));
                m_videoSettings.setBitRate(2000000);
            }
            break;
        case EVideoQualityAndBitRate:
            if (quality == QtMultimediaKit::VeryLowQuality) {
                m_videoSettings.setResolution(QSize(128,96));
                m_videoSettings.setFrameRate(10);
            } else if (quality == QtMultimediaKit::LowQuality) {
                m_videoSettings.setResolution(QSize(176,144));
                m_videoSettings.setFrameRate(15);
            } else if (quality == QtMultimediaKit::NormalQuality) {
                m_videoSettings.setResolution(QSize(160,120));
                m_videoSettings.setFrameRate(15);
            } else if (quality == QtMultimediaKit::HighQuality) {
                m_videoSettings.setResolution(QSize(352,288));
                m_videoSettings.setFrameRate(15);
            } else if (quality == QtMultimediaKit::VeryHighQuality) {
                m_videoSettings.setResolution(QSize(640,480));
                m_videoSettings.setFrameRate(15);
            }
            break;
        case EVideoQualityAndResolutionAndBitRate:
            if (quality == QtMultimediaKit::VeryLowQuality)
                m_videoSettings.setFrameRate(10);
            else if (quality == QtMultimediaKit::LowQuality)
                m_videoSettings.setFrameRate(15);
            else if (quality == QtMultimediaKit::NormalQuality)
                m_videoSettings.setFrameRate(15);
            else if (quality == QtMultimediaKit::HighQuality)
                m_videoSettings.setFrameRate(15);
            else if (quality == QtMultimediaKit::VeryHighQuality)
                m_videoSettings.setFrameRate(15);
            break;
        case EVideoQualityAndResolutionAndFrameRate:
            if (quality == QtMultimediaKit::VeryLowQuality)
                m_videoSettings.setBitRate(64000);
            else if (quality == QtMultimediaKit::LowQuality)
                m_videoSettings.setBitRate(64000);
            else if (quality == QtMultimediaKit::NormalQuality)
                m_videoSettings.setBitRate(128000);
            else if (quality == QtMultimediaKit::HighQuality)
                m_videoSettings.setBitRate(384000);
            else if (quality == QtMultimediaKit::VeryHighQuality)
                m_videoSettings.setBitRate(2000000);
            break;
        case EVideoQualityAndFrameRateAndBitRate:
            if (quality == QtMultimediaKit::VeryLowQuality)
                m_videoSettings.setResolution(QSize(128,96));
            else if (quality == QtMultimediaKit::LowQuality)
                m_videoSettings.setResolution(QSize(176,144));
            else if (quality == QtMultimediaKit::NormalQuality)
                m_videoSettings.setResolution(QSize(160,120));
            else if (quality == QtMultimediaKit::HighQuality)
                m_videoSettings.setResolution(QSize(352,288));
            else if (quality == QtMultimediaKit::VeryHighQuality)
                m_videoSettings.setResolution(QSize(640,480));
            break;
    }
#endif // S60_3X_PLATFORM

    m_videoSettings.setQuality(quality);

    m_uncommittedSettings = true;
}

void S60VideoCaptureSession::setAudioCaptureQuality(const QtMultimediaKit::EncodingQuality quality,
                                                    const AudioQualityDefinition mode)
{
    // Based on audio quality definition mode, select proper SampleRate and BitRate
    switch (mode) {
        case EOnlyAudioQuality:
            switch (quality) {
                case QtMultimediaKit::VeryLowQuality:
                    m_audioSettings.setBitRate(16000);
                    m_audioSettings.setSampleRate(8000);
                    break;
                case QtMultimediaKit::LowQuality:
                    m_audioSettings.setBitRate(32000);
                    m_audioSettings.setSampleRate(16000);
                    break;
                case QtMultimediaKit::NormalQuality:
                    m_audioSettings.setBitRate(64000);
                    m_audioSettings.setSampleRate(22050);
                    break;
                case QtMultimediaKit::HighQuality:
                    m_audioSettings.setBitRate(128000);
                    m_audioSettings.setSampleRate(44100);
                    break;
                case QtMultimediaKit::VeryHighQuality:
                    m_audioSettings.setBitRate(256000);
                    m_audioSettings.setSampleRate(48000);
                    break;
                default:
                    setError(KErrGeneral, QString("Unsupported quality."));
            }
            break;
        case EAudioQualityAndBitRate:
            switch (quality) {
                case QtMultimediaKit::VeryLowQuality:
                    m_audioSettings.setSampleRate(8000);
                    break;
                case QtMultimediaKit::LowQuality:
                    m_audioSettings.setSampleRate(16000);
                    break;
                case QtMultimediaKit::NormalQuality:
                    m_audioSettings.setSampleRate(22050);
                    break;
                case QtMultimediaKit::HighQuality:
                    m_audioSettings.setSampleRate(44100);
                    break;
                case QtMultimediaKit::VeryHighQuality:
                    m_audioSettings.setSampleRate(48000);
                    break;
                default:
                    setError(KErrGeneral, QString("Unsupported quality."));
            }
            break;
        case EAudioQualityAndSampleRate:
            switch (quality) {
                case QtMultimediaKit::VeryLowQuality:
                    m_audioSettings.setBitRate(16000);
                    break;
                case QtMultimediaKit::LowQuality:
                    m_audioSettings.setBitRate(32000);
                    break;
                case QtMultimediaKit::NormalQuality:
                    m_audioSettings.setBitRate(64000);
                    break;
                case QtMultimediaKit::HighQuality:
                    m_audioSettings.setBitRate(128000);
                    break;
                case QtMultimediaKit::VeryHighQuality:
                    m_audioSettings.setBitRate(256000);
                    break;
                default:
                    setError(KErrGeneral, QString("Unsupported quality."));
            }
            break;
        case ENoAudioQuality:
            // No actions required, just set quality parameter
            break;

        default:
            setError(KErrGeneral, QString("Unexpected camera error."));
    }

    m_audioSettings.setQuality(quality);
    m_uncommittedSettings = true;
}

int S60VideoCaptureSession::initializeVideoRecording()
{
    if (m_error) {
        return m_error;
    }

    TRAPD(symbianError, doInitializeVideoRecorderL());
    setError(symbianError, QString("Failed to initialize video recorder."));

    return symbianError;
}

void S60VideoCaptureSession::releaseVideoRecording()
{
    if (m_captureState >= ERecording) {
        m_videoRecorder->Stop();
    }

    if (m_captureState >= EInitialized) {
        m_videoRecorder->Close();
    }

    // Reset state
    m_captureState = ENotInitialized;

    // Reset error to be able to recover from error
    m_error = KErrNone;

    // Reset flags
    m_openWhenReady = false;
    m_prepareAfterOpenComplete = false;
    m_startAfterPrepareComplete = false;
    m_uncommittedSettings = false;
    m_commitSettingsWhenReady = false;
}

void S60VideoCaptureSession::startRecording()
{
    if (m_error) {
        setError(m_error, QString("Unexpected recording error."));
        return;
    }

    if (!m_cameraStarted) {
        setError(KErrNotReady, QString("Camera is not started."));
        return;
    }

    switch (m_captureState) {
        case ENotInitialized:
        case EInitializing:
        case EInitialized:
            if (m_captureState == EInitialized) {
                setOutputLocation(m_requestedSink);
            }
            m_startAfterPrepareComplete = true;
            return;

        case EOpening:
        case EPreparing:
            // Execute FileOpenL() and Prepare() asap and then start recording
            m_startAfterPrepareComplete = true;
            return;
        case EOpenComplete:
        case EPrepared:
            if (m_captureState == EPrepared && !m_uncommittedSettings) {
                break;
            }

            // Revert state internally, since logically applying settings means going
            // from OpenComplete ==> Preparing ==> Prepared.
            m_captureState = EOpenComplete;
            commitVideoEncoderSettings();

            // Start preparing
            m_captureState = EPreparing;
            emit stateChanged(m_captureState);

            if (m_cameraEngine->IsCameraReady()) {
                m_startAfterPrepareComplete = true;
                m_videoRecorder->Prepare();
            }
            return;
        case ERecording:
            // Discard
            return;
        case EPaused:
            // Continue
            break;

        default:
            setError(KErrGeneral, QString("Unexpected camera error."));
            return;
    }

    // State should now be either Prepared with no Uncommitted Settings or Paused

    if (m_cameraEngine && !m_cameraEngine->IsCameraReady()) {
        setError(KErrNotReady, QString("Camera not ready to start video recording."));
        return;
    }

    if (m_videoRecorder) {
        m_videoRecorder->Record();
        m_captureState = ERecording;
        emit stateChanged(m_captureState);
    }
    else
        setError(KErrNotReady, QString("Unexpected camera error."));
}

void S60VideoCaptureSession::pauseRecording()
{
    if (m_captureState == ERecording) {
        if (m_videoRecorder) {
            TRAPD(err, m_videoRecorder->PauseL());
            setError(err, QString("Pausing video recording failed."));
            m_captureState = EPaused;
            emit stateChanged(m_captureState);
        }
        else
            setError(KErrNotReady, QString("Unexpected camera error."));

    }
}

void S60VideoCaptureSession::stopRecording(const bool reInitialize)
{
    if (m_captureState != ERecording && m_captureState != EPaused)
        return; // Ignore

    if (m_videoRecorder) {
        m_videoRecorder->Stop();
        m_videoRecorder->Close();

        m_captureState = ENotInitialized;
        emit stateChanged(m_captureState);

        // VideoRecording will be re-initialized unless explicitly requested not to do so
        if (reInitialize) {
            if (m_cameraEngine->IsCameraReady()) {
                initializeVideoRecording();
            }
        }
    }
    else
        setError(KErrNotReady, QString("Unexpected camera error."));
}

void S60VideoCaptureSession::updateVideoCaptureContainers()
{
    TRAPD(err, doUpdateVideoCaptureContainersL());
    setError(err, QString("Failed to gather video container information."));
}

void S60VideoCaptureSession::doUpdateVideoCaptureContainersL()
{
    // Clear container data structure
    QList<TInt> mapControllers = m_videoControllerMap.keys();
    for (int i = 0; i < mapControllers.size(); ++i) {
        foreach(VideoFormatData data, m_videoControllerMap[mapControllers[i]]){
            data.supportedMimeTypes.clear();
        }
        m_videoControllerMap[mapControllers[i]].clear();
    }
    m_videoControllerMap.clear();

    // Resolve the supported video format and retrieve a list of controllers
    CMMFControllerPluginSelectionParameters* pluginParameters =
        CMMFControllerPluginSelectionParameters::NewLC();
    CMMFFormatSelectionParameters* format =
        CMMFFormatSelectionParameters::NewLC();

    // Set the play and record format selection parameters to be blank.
    // Format support is only retrieved if requested.
    pluginParameters->SetRequiredPlayFormatSupportL(*format);
    pluginParameters->SetRequiredRecordFormatSupportL(*format);

    // Set the media IDs
    RArray<TUid> mediaIds;
    CleanupClosePushL(mediaIds);

    User::LeaveIfError(mediaIds.Append(KUidMediaTypeVideo));

    // Get plugins that support at least video
    pluginParameters->SetMediaIdsL(mediaIds,
        CMMFPluginSelectionParameters::EAllowOtherMediaIds);
    pluginParameters->SetPreferredSupplierL(KNullDesC,
        CMMFPluginSelectionParameters::EPreferredSupplierPluginsFirstInList);

    // Array to hold all the controllers support the match data
    RMMFControllerImplInfoArray controllers;
    CleanupResetAndDestroyPushL(controllers);
    pluginParameters->ListImplementationsL(controllers);

    // Find the first controller with at least one record format available
    for (TInt index = 0; index < controllers.Count(); ++index) {
        m_videoControllerMap.insert(controllers[index]->Uid().iUid, QHash<TInt,VideoFormatData>());

        const RMMFFormatImplInfoArray& recordFormats = controllers[index]->RecordFormats();
        for (TInt j = 0; j < recordFormats.Count(); ++j) {
            VideoFormatData formatData;
            formatData.description = QString::fromUtf16(
                                    recordFormats[j]->DisplayName().Ptr(),
                                    recordFormats[j]->DisplayName().Length());

            const CDesC8Array& mimeTypes = recordFormats[j]->SupportedMimeTypes();
            for (int k = 0; k < mimeTypes.Count(); ++k) {
                TPtrC8 mimeType = mimeTypes[k];
                QString type = QString::fromUtf8((char *)mimeType.Ptr(),
                        mimeType.Length());
                formatData.supportedMimeTypes.append(type);
            }

            m_videoControllerMap[controllers[index]->Uid().iUid].insert(recordFormats[j]->Uid().iUid, formatData);
        }

    }

    CleanupStack::PopAndDestroy(&controllers);
    CleanupStack::PopAndDestroy(&mediaIds);
    CleanupStack::PopAndDestroy(format);
    CleanupStack::PopAndDestroy(pluginParameters);
}

/*
 * This goes through the available controllers and selects proper one based
 * on the format. Function sets proper UIDs to be used for controller and format.
 */
void S60VideoCaptureSession::selectController(const QString &format,
                                              TUid &controllerUid,
                                              TUid &formatUid)
{
    QList<TInt> controllers = m_videoControllerMap.keys();
    QList<TInt> formats;

    for (int i = 0; i < controllers.count(); ++i) {
        formats = m_videoControllerMap[controllers[i]].keys();
        for (int j = 0; j < formats.count(); ++j) {
            VideoFormatData formatData = m_videoControllerMap[controllers[i]][formats[j]];
            if (formatData.supportedMimeTypes.contains(format, Qt::CaseInsensitive)) {
                controllerUid = TUid::Uid(controllers[i]);
                formatUid = TUid::Uid(formats[j]);
            }
        }
    }
}

QStringList S60VideoCaptureSession::supportedVideoCaptureCodecs()
{
    return m_videoCodeclist;
}

QStringList S60VideoCaptureSession::supportedAudioCaptureCodecs()
{
    QStringList keys = m_audioCodeclist.keys();
    keys.sort();
    return keys;
}

QList<int> S60VideoCaptureSession::supportedSampleRates(const QAudioEncoderSettings &settings, bool *continuous)
{
    QList<int> rates;

    TRAPD(err, rates = doGetSupportedSampleRatesL(settings, continuous));
    if (err != KErrNotSupported)
        setError(err, QString("Failed to query information of supported sample rates."));

    return rates;
}

QList<int> S60VideoCaptureSession::doGetSupportedSampleRatesL(const QAudioEncoderSettings &settings, bool *continuous)
{
    QList<int> sampleRates;

#ifndef S60_31_PLATFORM
    RArray<TUint> supportedSampleRates;
    CleanupClosePushL(supportedSampleRates);

    if (!settings.codec().isEmpty()) {
        TFourCC currentAudioCodec;
        currentAudioCodec = m_videoRecorder->AudioTypeL();

        TFourCC requestedAudioCodec;
        if (qstrcmp(settings.codec().toLocal8Bit().constData(), "audio/aac") == 0)
            requestedAudioCodec.Set(KMMFFourCCCodeAAC);
        else if (qstrcmp(settings.codec().toLocal8Bit().constData(), "audio/amr") == 0)
            requestedAudioCodec.Set(KMMFFourCCCodeAMR);
        m_videoRecorder->SetAudioTypeL(requestedAudioCodec);

        m_videoRecorder->GetSupportedAudioSampleRatesL(supportedSampleRates);

        m_videoRecorder->SetAudioTypeL(currentAudioCodec);
    }
    else
        m_videoRecorder->GetSupportedAudioSampleRatesL(supportedSampleRates);

    for (int i = 0; i < supportedSampleRates.Count(); ++i) {
        sampleRates.append((int)supportedSampleRates[i]);
    }

    CleanupStack::PopAndDestroy(); // RArray<TUint> supportedSampleRates
#else // S60 3.1 Platform
    Q_UNUSED(settings);
#endif // S60_31_PLATFORM

    if (continuous)
        *continuous = false;

    return sampleRates;
}

void S60VideoCaptureSession::setAudioSampleRate(const int sampleRate)
{
    if (sampleRate != -1)
        m_audioSettings.setSampleRate(sampleRate);

    m_uncommittedSettings = true;
}

void S60VideoCaptureSession::setAudioBitRate(const int bitRate)
{
    if (bitRate != -1)
        m_audioSettings.setBitRate(bitRate);

    m_uncommittedSettings = true;
}

void S60VideoCaptureSession::setAudioChannelCount(const int channelCount)
{
    if (channelCount != -1)
        m_audioSettings.setChannelCount(channelCount);

    m_uncommittedSettings = true;
}

void S60VideoCaptureSession::setVideoCaptureCodec(const QString &codecName)
{
    if (codecName.isEmpty() || codecName == m_videoSettings.codec())
        return;

    m_videoSettings.setCodec(codecName);

    m_uncommittedSettings = true;
}

void S60VideoCaptureSession::setAudioCaptureCodec(const QString &codecName)
{
    if (codecName.isEmpty() || codecName == m_audioSettings.codec())
        return;

    m_audioSettings.setCodec(codecName);

    m_uncommittedSettings = true;
}

QString S60VideoCaptureSession::videoCaptureCodecDescription(const QString &codecName)
{
    QString codecDescription;
    if (codecName.contains("video/H263-2000", Qt::CaseInsensitive))
        codecDescription.append("H.263 Video Codec");
    else if (codecName.contains("video/mp4v-es", Qt::CaseInsensitive))
        codecDescription.append("MPEG-4 Part 2 Video Codec");
    else if (codecName.contains("video/H264", Qt::CaseInsensitive))
        codecDescription.append("H.264 AVC (MPEG-4 Part 10) Video Codec");
    else
        codecDescription.append("Video Codec");

    return codecDescription;
}

void S60VideoCaptureSession::doSetCodecsL(const QString &aCodec, const QString &vCodec)
{
    if (m_videoRecorder) {
        TPtrC16 str(reinterpret_cast<const TUint16*>(m_videoSettings.codec().utf16()));
        HBufC8* videoCodec(NULL);
        videoCodec = CnvUtfConverter::ConvertFromUnicodeToUtf8L(str);
        CleanupStack::PushL(videoCodec);

        TFourCC audioCodec = m_audioCodeclist[m_audioSettings.codec()];

        m_videoRecorder->SetVideoTypeL(*videoCodec);

        m_videoRecorder->SetAudioTypeL(audioCodec);

        CleanupStack::PopAndDestroy(videoCodec);
    }
    else
        setError(KErrNotReady, QString("Unexpected camera error."));
}

void S60VideoCaptureSession::setBitrate(const int bitrate)
{
    m_videoSettings.setBitRate(bitrate);

    m_uncommittedSettings = true;
}

void S60VideoCaptureSession::doSetBitrate(const int &bitrate)
{
    if (bitrate != -1) {
        if (m_videoRecorder) {
            TRAPD(err, m_videoRecorder->SetVideoBitRateL(bitrate));
            if (err) {
                setError(err, QString("Failed to set video bitrate."));
            }
        } else {
            setError(KErrNotReady, QString("Unexpected camera error."));
        }
    }
}

void S60VideoCaptureSession::setVideoResolution(const QSize &resolution)
{
    m_videoSettings.setResolution(resolution);

    m_uncommittedSettings = true;
}

void S60VideoCaptureSession::doSetVideoResolution(const QSize &resolution)
{
    TSize size((TInt)resolution.width(), (TInt)resolution.height());

    // Make sure resolution is not too big if main camera is not used
    if (m_cameraEngine->currentCameraIndex() != 0) {
        TCameraInfo *info = m_cameraEngine->cameraInfo();
        if (info) {
            TInt videoResolutionCount = info->iNumVideoFrameSizesSupported;
            TSize maxCameraVideoResolution = TSize(0,0);
            CCamera *camera = m_cameraEngine->Camera();
            if (camera) {
                for (TInt i = 0; i < videoResolutionCount; ++i) {
                    TSize checkedResolution;
                    // Use YUV video max frame size in the check (Through
                    // CVideoRecorderUtility/DevVideoRecord it is possible to
                    // query only encoder maximums)
                    camera->EnumerateVideoFrameSizes(checkedResolution, i, CCamera::EFormatYUV420Planar);
                    if ((checkedResolution.iWidth * checkedResolution.iHeight) >
                        (maxCameraVideoResolution.iWidth * maxCameraVideoResolution.iHeight))
                        maxCameraVideoResolution = checkedResolution;
                }
                if ((maxCameraVideoResolution.iWidth * maxCameraVideoResolution.iHeight) <
                    (size.iWidth * size.iHeight))
                    size = maxCameraVideoResolution;
            }
            else
                setError(KErrGeneral, QString("Could not query supported video resolutions."));
        }else
            setError(KErrGeneral, QString("Could not query supported video resolutions."));
    }
    if (resolution.width() != -1 && resolution.height() != -1) {
        if (m_videoRecorder) {
            TRAPD(err, m_videoRecorder->SetVideoFrameSizeL((TSize)size));
            if (err == KErrNotSupported) {
                TSize fallBack(640,480);
                TRAPD(err, m_videoRecorder->SetVideoFrameSizeL(fallBack));
                if (err == KErrNone)
                    m_videoSettings.setResolution(QSize(fallBack.iWidth,fallBack.iHeight));
                else {
                    fallBack = TSize(176,144);
                    TRAPD(err, m_videoRecorder->SetVideoFrameSizeL(fallBack));
                    if (err == KErrNone)
                        m_videoSettings.setResolution(QSize(fallBack.iWidth,fallBack.iHeight));
                    else
                        setError(err, QString("Failed to set fallback resolution 176x144."));
                }
            }
            else
                setError(err, QString("Failed to set video resolution."));
        } else {
            setError(KErrNotReady, QString("Unexpected camera error."));
        }
    }
}

void S60VideoCaptureSession::setFrameRate(qreal rate)
{
    m_videoSettings.setFrameRate(rate);

    m_uncommittedSettings = true;
}

void S60VideoCaptureSession::doSetFrameRate(qreal rate)
{
    if (rate != 0) {
        if (m_videoRecorder) {
            bool continuous = false;
            QList<qreal> list = supportedVideoFrameRates(&continuous);
            qreal maxRate = 0.0;
            foreach (qreal fRate, list)
                if (fRate > maxRate)
                    maxRate = fRate;
            if (maxRate >= rate) {
                TRAPD(err, m_videoRecorder->SetVideoFrameRateL((TReal32)rate));
                if (err == KErrNotSupported) {
                    TReal32 fallBack = 15.0;
                    TRAPD(err, m_videoRecorder->SetVideoFrameRateL(fallBack));
                    if (err == KErrNone)
                        m_videoSettings.setFrameRate((qreal)fallBack);
                    else
                        setError(err, QString("Failed to set fallback framerate 15 fps."));
                }
                else
                    setError(err, QString("Failed to set video framerate."));
            } else {
                setError(KErrNotSupported, QString("Framerate is not supported."));
            }
        } else {
            setError(KErrNotReady, QString("Unexpected camera error."));
        }
    }
}

void S60VideoCaptureSession::setVideoEncodingMode(const QtMultimediaKit::EncodingMode mode)
{
    // This has no effect as it has no support in Symbian
    m_videoSettings.setEncodingMode(mode);

    // m_uncommittedSettings = true;
}

void S60VideoCaptureSession::setAudioEncodingMode(const QtMultimediaKit::EncodingMode mode)
{
    // This has no effect as it has no support in Symbian
    m_audioSettings.setEncodingMode(mode);

    // m_uncommittedSettings = true;
}

void S60VideoCaptureSession::initializeVideoCaptureSettings()
{
    // Codecs - Use ones defined in constants if supported
    if (m_videoCodeclist.count() > 0) {
        if (m_videoCodeclist.contains(KMimeTypeDefaultVideoCodec, Qt::CaseInsensitive)) {
            m_videoSettings.setCodec(KMimeTypeDefaultVideoCodec);
        } else {
            if (m_videoCodeclist.size() > 0)
                m_videoSettings.setCodec(m_videoCodeclist[m_videoCodeclist.size()-1]);
        }
    }

    QStringList aCodecs = m_audioCodeclist.keys();
    if (aCodecs.count() > 0) {
        if (aCodecs.contains(KMimeTypeDefaultAudioCodec, Qt::CaseInsensitive)) {
            m_audioSettings.setCodec(KMimeTypeDefaultAudioCodec);
        } else {
            if (aCodecs.size() > 0)
                m_audioSettings.setCodec(aCodecs[0]);
        }
    }

    // Video Settings
    m_videoSettings.setBitRate(maximumBitRateForMimeType(m_videoSettings.codec())); // Use max supported
    m_videoSettings.setEncodingMode(QtMultimediaKit::AverageBitRateEncoding);
    m_videoSettings.setQuality(QtMultimediaKit::VeryHighQuality);

    // Use maximum resolution and framerate supported
    bool continuous = false;
    QList<QSize> sizes = supportedVideoResolutions(m_videoSettings, &continuous);
    QSize maxSize = QSize();
    foreach (QSize size, sizes) {
        if ((size.width() * size.height()) > (maxSize.width() * maxSize.height()))
            maxSize = size;
    }
    m_videoSettings.setResolution(maxSize);

    QList<qreal> rates = supportedVideoFrameRates(m_videoSettings, &continuous);
    qreal maxRate = 0.0;
    foreach (qreal rate, rates)
        maxRate = qMax(maxRate, rate);
    m_videoSettings.setFrameRate(maxRate);

    // Audio Settings
    m_audioSettings.setEncodingMode(QtMultimediaKit::AverageBitRateEncoding);
    m_audioSettings.setBitRate(KDefaultBitRate);
    m_audioSettings.setChannelCount(KDefaultChannelCount);
    m_audioSettings.setQuality(QtMultimediaKit::VeryHighQuality);

    QList<int> sampleRates = supportedSampleRates(m_audioSettings, &continuous);
    if (sampleRates.count() > 0) {
        if (sampleRates.indexOf(KDefaultSampleRate) != -1)
            m_audioSettings.setSampleRate(KDefaultSampleRate);
        else
            m_audioSettings.setSampleRate(sampleRates[0]);
    }
    else
        m_audioSettings.setSampleRate(-1); // Setting SampleRate is not supported

}

QSize S60VideoCaptureSession::pixelAspectRatio()
{
#ifndef S60_31_PLATFORM
    TVideoAspectRatio par;
    TRAPD(err, m_videoRecorder->GetPixelAspectRatioL(par));
    if (err) {
        setError(err, QString("Failed to query current pixel aspect ratio."));
    }
    return QSize(par.iNumerator, par.iDenominator);
#else // S60_31_PLATFORM
    return QSize();
#endif // !S60_31_PLATFORM
}

void S60VideoCaptureSession::setPixelAspectRatio(const QSize par)
{
#ifndef S60_31_PLATFORM
    const TVideoAspectRatio videoPar(par.width(), par.height());
    TRAPD(err, m_videoRecorder->SetPixelAspectRatioL(videoPar));
    if (err) {
        setError(err, QString("Failed to set pixel aspect ratio."));
    }
#else // S60_31_PLATFORM
    Q_UNUSED(par);
#endif // !S60_31_PLATFORM

    m_uncommittedSettings = true;
}

int S60VideoCaptureSession::gain()
{
    TInt gain = 0;
    TRAPD(err, gain = m_videoRecorder->GainL());
    if (err) {
        setError(err, QString("Failed to query video gain."));
    }
    return (int)gain;
}

void S60VideoCaptureSession::setGain(const int gain)
{
    TRAPD(err, m_videoRecorder->SetGainL(gain));
    if (err) {
        setError(err, QString("Failed to set video gain."));
    }

    m_uncommittedSettings = true;
}

int S60VideoCaptureSession::maxClipSizeInBytes() const
{
    return m_maxClipSize;
}

void S60VideoCaptureSession::setMaxClipSizeInBytes(const int size)
{
    TRAPD(err, m_videoRecorder->SetMaxClipSizeL(size));
    if (err) {
        setError(err, QString("Failed to set maximum video size."));
    } else
        m_maxClipSize = size;

    m_uncommittedSettings = true;
}

void S60VideoCaptureSession::MvruoOpenComplete(TInt aError)
{
    if (m_error)
        return;

    if (aError == KErrNone && m_videoRecorder) {
        if (m_captureState == EInitializing) {
            // Dummy file open completed, initialize settings
            TRAPD(err, doPopulateAudioCodecsL());
            setError(err, QString("Failed to gather information of supported audio codecs."));

            // For DevVideoRecord codecs are populated during
            // doPopulateVideoCodecsDataL()
            TRAP(err, doPopulateVideoCodecsL());
            setError(err, QString("Failed to gather information of supported video codecs."));
#ifndef S60_DEVVIDEO_RECORDING_SUPPORTED
            // Max parameters needed to be populated, if not using DevVideoRecord
            // Otherwise done already in constructor
            doPopulateMaxVideoParameters();
#endif

            initializeVideoCaptureSettings();

            m_captureState = EInitialized;
            emit stateChanged(m_captureState);

            if (m_openWhenReady == true) {
                setOutputLocation(m_requestedSink);
                m_openWhenReady = false; // Reset
            }
            if (m_commitSettingsWhenReady) {
                applyAllSettings();
                m_commitSettingsWhenReady = false; // Reset
            }

            return;

        } else if (m_captureState == EOpening) {
            // Actual file open completed
            m_captureState = EOpenComplete;
            emit stateChanged(m_captureState);

            // Prepare right away
            if (m_startAfterPrepareComplete || m_prepareAfterOpenComplete) {
                m_prepareAfterOpenComplete = false; // Reset
                commitVideoEncoderSettings();
                if (m_cameraEngine->IsCameraReady()) {
                    m_videoRecorder->Prepare();
                }
            }
            return;

        } else if (m_captureState == ENotInitialized) {
            // Resources released while waiting OpenFileL to complete
            m_videoRecorder->Close();
            return;

        } else {
            setError(KErrGeneral, QString("Unexpected camera error."));
            return;
        }
    }

    m_videoRecorder->Close();
    setError(aError, QString("Failure during video recorder initialization."));
}

void S60VideoCaptureSession::MvruoPrepareComplete(TInt aError)
{
    if (m_error)
        return;

    if(aError == KErrNone) {
        if (m_captureState == ENotInitialized) {
            // Resources released while waiting for Prepare to complete
            m_videoRecorder->Close();
            return;
        }

        m_captureState = EPrepared;
        emit stateChanged(EPrepared);

        if (m_openWhenReady == true) {
            setOutputLocation(m_requestedSink);
            m_openWhenReady = false; // Reset
        }

        if (m_commitSettingsWhenReady) {
            applyAllSettings();
            m_commitSettingsWhenReady = false; // Reset
        }

        if (m_startAfterPrepareComplete) {
            m_startAfterPrepareComplete = false; // Reset
            startRecording();
        }
    } else {
        m_videoRecorder->Close();
        setError(aError, QString("Failed to prepare camera for video recording."));
    }
}

void S60VideoCaptureSession::MvruoRecordComplete(TInt aError)
{
    if (!m_videoRecorder) {
        setError(KErrNotReady, QString("Unexpected camera error."));
        return;
    }

    if((aError == KErrNone || aError == KErrCompletion)) {
        m_videoRecorder->Stop();

        // Reset state
        if (m_captureState != ENotInitialized) {
            m_captureState = ENotInitialized;
            emit stateChanged(m_captureState);
        }

        if (m_cameraEngine->IsCameraReady()) {
            initializeVideoRecording();
        }
    }
    m_videoRecorder->Close();

    if (aError == KErrDiskFull)
        setError(aError, QString("Not enough space for video, recording stopped."));
    else
        setError(aError, QString("Recording stopped due to unexpected error."));
}

void S60VideoCaptureSession::MvruoEvent(const TMMFEvent& aEvent)
{
    Q_UNUSED(aEvent);
}

#ifdef S60_DEVVIDEO_RECORDING_SUPPORTED
void S60VideoCaptureSession::MdvroReturnPicture(TVideoPicture *aPicture)
{
    // Not used
    Q_UNUSED(aPicture);
}

void S60VideoCaptureSession::MdvroSupplementalInfoSent()
{
    // Not used
}

void S60VideoCaptureSession::MdvroNewBuffers()
{
    // Not used
}

void S60VideoCaptureSession::MdvroFatalError(TInt aError)
{
    setError(aError, QString("Unexpected camera error."));
}

void S60VideoCaptureSession::MdvroInitializeComplete(TInt aError)
{
    // Not used
    Q_UNUSED(aError);
}

void S60VideoCaptureSession::MdvroStreamEnd()
{
    // Not used
}

/*
 * This populates video codec information (supported codecs, resolutions,
 * framerates, etc.) using DevVideoRecord API.
 */
void S60VideoCaptureSession::doPopulateVideoCodecsDataL()
{
    RArray<TUid> encoders;
    CleanupClosePushL(encoders);

    CMMFDevVideoRecord *mDevVideoRecord = CMMFDevVideoRecord::NewL(*this);
    CleanupStack::PushL(mDevVideoRecord);

    // Retrieve list of all encoders provided by the platform
    mDevVideoRecord->GetEncoderListL(encoders);

    for (int i = 0; i < encoders.Count(); ++i ) {
        CVideoEncoderInfo *encoderInfo = mDevVideoRecord->VideoEncoderInfoLC(encoders[i]);

        // Discard encoders that are not HW accelerated and do not support direct capture
        if (encoderInfo->Accelerated() == false || encoderInfo->SupportsDirectCapture() == false) {
            CleanupStack::Check(encoderInfo);
            CleanupStack::PopAndDestroy(encoderInfo);
            continue;
        }

        m_videoParametersForEncoder.append(MaxResolutionRatesAndTypes());
        int newIndex = m_videoParametersForEncoder.count() - 1;

        m_videoParametersForEncoder[newIndex].bitRate = (int)encoderInfo->MaxBitrate();

        // Get supported MIME Types
        const RPointerArray<CCompressedVideoFormat> &videoFormats = encoderInfo->SupportedOutputFormats();
        for(int x = 0; x < videoFormats.Count(); ++x) {
            QString codecMimeType = QString::fromUtf8((char *)videoFormats[x]->MimeType().Ptr(),videoFormats[x]->MimeType().Length());

            //m_videoCodeclist.append(codecMimeType);
            m_videoParametersForEncoder[newIndex].mimeTypes.append(codecMimeType);
        }

        // Get supported maximum Resolution/Framerate pairs
        const RArray<TPictureRateAndSize> &ratesAndSizes = encoderInfo->MaxPictureRates();
        SupportedFrameRatePictureSize data;
        for(int j = 0; j < ratesAndSizes.Count(); ++j) {
            data.frameRate = ratesAndSizes[j].iPictureRate;
            data.frameSize = QSize(ratesAndSizes[j].iPictureSize.iWidth, ratesAndSizes[j].iPictureSize.iHeight);

            // Save data to the hash
            m_videoParametersForEncoder[newIndex].frameRatePictureSizePair.append(data);
        }

        CleanupStack::Check(encoderInfo);
        CleanupStack::PopAndDestroy(encoderInfo);
    }

    CleanupStack::Check(mDevVideoRecord);
    CleanupStack::PopAndDestroy(mDevVideoRecord);
    CleanupStack::PopAndDestroy(); // RArray<TUid> encoders
}
#endif  // S60_DEVVIDEO_RECORDING_SUPPORTED

QStringList S60VideoCaptureSession::supportedVideoContainers()
{
    QStringList containers;

    QList<TInt> controllers = m_videoControllerMap.keys();
    for (int i = 0; i < controllers.count(); ++i) {
        foreach (VideoFormatData formatData, m_videoControllerMap[controllers[i]]) {
            for (int j = 0; j < formatData.supportedMimeTypes.count(); ++j) {
                if (containers.contains(formatData.supportedMimeTypes[j], Qt::CaseInsensitive) == false) {
                    containers.append(formatData.supportedMimeTypes[j]);
                }
            }
        }
    }

    return containers;
}

bool S60VideoCaptureSession::isSupportedVideoContainer(const QString &containerName)
{
    return supportedVideoContainers().contains(containerName, Qt::CaseInsensitive);
}

QString S60VideoCaptureSession::videoContainer() const
{
    return m_container;
}

void S60VideoCaptureSession::setVideoContainer(const QString &containerName)
{
    if (containerName == m_requestedContainer)
        return;

    m_requestedContainer = containerName;
    m_uncommittedSettings = true;
}

QString S60VideoCaptureSession::videoContainerDescription(const QString &containerName)
{
    QList<TInt> formats;
    QList<TInt> encoders = m_videoControllerMap.keys();
    for (int i = 0; i < encoders.count(); ++i) {
        formats = m_videoControllerMap[encoders[i]].keys();
        for (int j = 0; j < formats.count(); ++j) {
            if (m_videoControllerMap[encoders[i]][formats[j]].supportedMimeTypes.contains(containerName, Qt::CaseInsensitive)) {
                return m_videoControllerMap[encoders[i]][formats[j]].description;
            }
        }
    }

    return QString();
}


void S60VideoCaptureSession::cameraStatusChanged(QCamera::Status status)
{
    if (status == QCamera::ActiveStatus) {
        m_cameraStarted = true;
        return;
    }
    else if (status == QCamera::UnloadedStatus)
        releaseVideoRecording();

    m_cameraStarted = false;
}

void S60VideoCaptureSession::doPopulateAudioCodecsL()
{
    if (m_captureState == EInitializing) {
        m_audioCodeclist.clear();

        RArray<TFourCC> audioTypes;
        CleanupClosePushL(audioTypes);

        if (m_videoRecorder)
            m_videoRecorder->GetSupportedAudioTypesL(audioTypes);
        else
            setError(KErrNotReady, QString("Unexpected camera error."));

        for (TInt i = 0; i < audioTypes.Count(); i++) {
            TUint32 codec = audioTypes[i].FourCC();
            if (codec == KMMFFourCCCodeAMR)
                m_audioCodeclist.insert(QString("audio/amr"), KMMFFourCCCodeAMR);
            if (codec == KMMFFourCCCodeAAC)
                m_audioCodeclist.insert(QString("audio/aac"), KMMFFourCCCodeAAC);
        }
        CleanupStack::PopAndDestroy(&audioTypes);
    }
}

void S60VideoCaptureSession::doPopulateVideoCodecsL()
{
    if (m_captureState == EInitializing) {
        m_videoCodeclist.clear();

        CDesC8ArrayFlat* videoTypes = new (ELeave) CDesC8ArrayFlat(10);
        CleanupStack::PushL(videoTypes);

        if (m_videoRecorder)
            m_videoRecorder->GetSupportedVideoTypesL(*videoTypes);
        else
            setError(KErrNotReady, QString("Unexpected camera error."));

        for (TInt i = 0; i < videoTypes->Count(); i++) {
            TPtrC8 videoType = videoTypes->MdcaPoint(i);
            QString codecMimeType = QString::fromUtf8((char *)videoType.Ptr(), videoType.Length());
#ifdef S60_DEVVIDEO_RECORDING_SUPPORTED
            for (int j = 0; j < m_videoParametersForEncoder.size(); ++j) {
                if (m_videoParametersForEncoder[j].mimeTypes.contains(codecMimeType, Qt::CaseInsensitive)) {
                    m_videoCodeclist << codecMimeType;
                    break;
                }
            }
#else // CVideoRecorderUtility
            m_videoCodeclist << codecMimeType;
#endif // S60_DEVVIDEO_RECORDING_SUPPORTED
        }
        CleanupStack::PopAndDestroy(videoTypes);
    }
}

#ifndef S60_DEVVIDEO_RECORDING_SUPPORTED
/*
 * Maximum resolution, framerate and bitrate can not be queried via MMF or
 * ECam, but needs to be set according to the definitions of the video
 * standard in question. In video standards, the values often depend on each
 * other, but the below function defines constant maximums.
 */
void S60VideoCaptureSession::doPopulateMaxVideoParameters()
{
    m_videoParametersForEncoder.append(MaxResolutionRatesAndTypes()); // For H.263
    m_videoParametersForEncoder.append(MaxResolutionRatesAndTypes()); // For MPEG-4
    m_videoParametersForEncoder.append(MaxResolutionRatesAndTypes()); // For H.264

    for (int i = 0; i < m_videoCodeclist.count(); ++i) {

        // Use all lower case for comparisons
        QString codec = m_videoCodeclist[i].toLower();

        if (codec.contains("video/h263-2000", Qt::CaseInsensitive)) {
            // H.263
            if (codec == "video/h263-2000" ||
                codec == "video/h263-2000; profile=0" ||
                codec == "video/h263-2000; profile=0; level=10" ||
                codec == "video/h263-2000; profile=3") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(15.0, QSize(176,144)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 64000)
                    m_videoParametersForEncoder[0].bitRate = 64000;
                continue;
            } else if (codec == "video/h263-2000; profile=0; level=20") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(15.0, QSize(352,288)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 128000)
                    m_videoParametersForEncoder[0].bitRate = 128000;
                continue;
            } else if (codec == "video/h263-2000; profile=0; level=30") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(30.0, QSize(352,288)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 384000)
                    m_videoParametersForEncoder[0].bitRate = 384000;
                continue;
            } else if (codec == "video/h263-2000; profile=0; level=40") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(30.0, QSize(352,288)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 2048000)
                    m_videoParametersForEncoder[0].bitRate = 2048000;
                continue;
            } else if (codec == "video/h263-2000; profile=0; level=45") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(15.0, QSize(176,144)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 128000)
                    m_videoParametersForEncoder[0].bitRate = 128000;
                continue;
            } else if (codec == "video/h263-2000; profile=0; level=50") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(15.0, QSize(352,288)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 4096000)
                    m_videoParametersForEncoder[0].bitRate = 4096000;
                continue;
            }

        } else if (codec.contains("video/mp4v-es", Qt::CaseInsensitive)) {
            // Mpeg-4
            if (codec == "video/mp4v-es" ||
                codec == "video/mp4v-es; profile-level-id=1" ||
                codec == "video/mp4v-es; profile-level-id=8") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(15.0, QSize(176,144)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 64000)
                    m_videoParametersForEncoder[0].bitRate = 64000;
                continue;
            } else if (codec == "video/mp4v-es; profile-level-id=2" ||
                       codec == "video/mp4v-es; profile-level-id=9") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(15.0, QSize(352,288)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 128000)
                    m_videoParametersForEncoder[0].bitRate = 128000;
                continue;
            } else if (codec == "video/mp4v-es; profile-level-id=3") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(30.0, QSize(352,288)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 384000)
                    m_videoParametersForEncoder[0].bitRate = 384000;
                continue;
            } else if (codec == "video/mp4v-es; profile-level-id=4") {
#if (defined(S60_31_PLATFORM) | defined(S60_32_PLATFORM))
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(15.0, QSize(640,480)));
#else // S60 5.0 and later platforms
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(30.0, QSize(640,480)));
#endif
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 4000000)
                    m_videoParametersForEncoder[0].bitRate = 4000000;
                continue;
            } else if (codec == "video/mp4v-es; profile-level-id=5") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(25.0, QSize(720,576)));
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(30.0, QSize(720,480)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 8000000)
                    m_videoParametersForEncoder[0].bitRate = 8000000;
                continue;
            } else if (codec == "video/mp4v-es; profile-level-id=6") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(30.0, QSize(1280,720)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 12000000)
                    m_videoParametersForEncoder[0].bitRate = 12000000;
                continue;
            }

        } else if (codec.contains("video/h264", Qt::CaseInsensitive)) {
            // H.264
            if (codec == "video/h264" ||
                codec == "video/h264; profile-level-id=42800a") {
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(15.0, QSize(176,144)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 64000)
                    m_videoParametersForEncoder[0].bitRate = 64000;
                continue;
            } else if (codec == "video/h264; profile-level-id=42900b") { // BP, L1b
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(15.0, QSize(176,144)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 128000)
                    m_videoParametersForEncoder[0].bitRate = 128000;
                continue;
            } else if (codec == "video/h264; profile-level-id=42800b") { // BP, L1.1
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(7.5, QSize(352,288)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 192000)
                    m_videoParametersForEncoder[0].bitRate = 192000;
                continue;
            } else if (codec == "video/h264; profile-level-id=42800c") { // BP, L1.2
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(15.0, QSize(352,288)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 384000)
                    m_videoParametersForEncoder[0].bitRate = 384000;
                continue;
            } else if (codec == "video/h264; profile-level-id=42800d") { // BP, L1.3
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(30.0, QSize(352,288)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 768000)
                    m_videoParametersForEncoder[0].bitRate = 768000;
                continue;
            } else if (codec == "video/h264; profile-level-id=428014") { // BP, L2
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(30.0, QSize(352,288)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 2000000)
                    m_videoParametersForEncoder[0].bitRate = 2000000;
                continue;
            } else if (codec == "video/h264; profile-level-id=428015") { // BP, L2.1
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(50.0, QSize(352,288)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 4000000)
                    m_videoParametersForEncoder[0].bitRate = 4000000;
                continue;
            } else if (codec == "video/h264; profile-level-id=428016") { // BP, L2.2
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(16.9, QSize(640,480)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 4000000)
                    m_videoParametersForEncoder[0].bitRate = 4000000;
                continue;
            } else if (codec == "video/h264; profile-level-id=42801e") { // BP, L3
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(33.8, QSize(640,480)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 10000000)
                    m_videoParametersForEncoder[0].bitRate = 10000000;
                continue;
            } else if (codec == "video/h264; profile-level-id=42801f") { // BP, L3.1
                m_videoParametersForEncoder[0].frameRatePictureSizePair.append(SupportedFrameRatePictureSize(30.0, QSize(1280,720)));
                m_videoParametersForEncoder[0].mimeTypes.append(codec);
                if (m_videoParametersForEncoder[0].bitRate < 14000000)
                    m_videoParametersForEncoder[0].bitRate = 14000000;
                continue;
            }
        }
    }
}
#endif // S60_DEVVIDEO_RECORDING_SUPPORTED

/*
 * This function returns the maximum resolution defined by the video standards
 * for different MIME Types.
 */
QSize S60VideoCaptureSession::maximumResolutionForMimeType(const QString &mimeType) const
{
    QSize maxSize(-1,-1);
    // Use all lower case for comparisons
    QString lowerMimeType = mimeType.toLower();

    if (lowerMimeType == "video/h263-2000") {
        maxSize = KResH263;
    } else if (lowerMimeType == "video/h263-2000; profile=0") {
        maxSize = KResH263_Profile0;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=10") {
        maxSize = KResH263_Profile0_Level10;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=20") {
        maxSize = KResH263_Profile0_Level20;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=30") {
        maxSize = KResH263_Profile0_Level30;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=40") {
        maxSize = KResH263_Profile0_Level40;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=45") {
        maxSize = KResH263_Profile0_Level45;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=50") {
        maxSize = KResH263_Profile0_Level50;
    } else if (lowerMimeType == "video/h263-2000; profile=3") {
        maxSize = KResH263_Profile3;
    } else if (lowerMimeType == "video/mp4v-es") {
        maxSize = KResMPEG4;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=1") {
        maxSize = KResMPEG4_PLID_1;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=2") {
        maxSize = KResMPEG4_PLID_2;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=3") {
        maxSize = KResMPEG4_PLID_3;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=4") {
        maxSize = KResMPEG4_PLID_4;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=5") {
        maxSize = KResMPEG4_PLID_5;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=6") {
        maxSize = KResMPEG4_PLID_6;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=8") {
        maxSize = KResMPEG4_PLID_8;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=9") {
        maxSize = KResMPEG4_PLID_9;
    } else if (lowerMimeType == "video/h264") {
        maxSize = KResH264;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800a" ||
               lowerMimeType == "video/h264; profile-level-id=4d400a" ||
               lowerMimeType == "video/h264; profile-level-id=64400a") { // L1
        maxSize = KResH264_PLID_42800A;
    } else if (lowerMimeType == "video/h264; profile-level-id=42900b" ||
               lowerMimeType == "video/h264; profile-level-id=4d500b" ||
               lowerMimeType == "video/h264; profile-level-id=644009") { // L1.b
        maxSize = KResH264_PLID_42900B;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800b" ||
               lowerMimeType == "video/h264; profile-level-id=4d400b" ||
               lowerMimeType == "video/h264; profile-level-id=64400b") { // L1.1
        maxSize = KResH264_PLID_42800B;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800c" ||
               lowerMimeType == "video/h264; profile-level-id=4d400c" ||
               lowerMimeType == "video/h264; profile-level-id=64400c") { // L1.2
        maxSize = KResH264_PLID_42800C;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800d" ||
               lowerMimeType == "video/h264; profile-level-id=4d400d" ||
               lowerMimeType == "video/h264; profile-level-id=64400d") { // L1.3
        maxSize = KResH264_PLID_42800D;
    } else if (lowerMimeType == "video/h264; profile-level-id=428014" ||
               lowerMimeType == "video/h264; profile-level-id=4d4014" ||
               lowerMimeType == "video/h264; profile-level-id=644014") { // L2
        maxSize = KResH264_PLID_428014;
    } else if (lowerMimeType == "video/h264; profile-level-id=428015" ||
               lowerMimeType == "video/h264; profile-level-id=4d4015" ||
               lowerMimeType == "video/h264; profile-level-id=644015") { // L2.1
        maxSize = KResH264_PLID_428015;
    } else if (lowerMimeType == "video/h264; profile-level-id=428016" ||
               lowerMimeType == "video/h264; profile-level-id=4d4016" ||
               lowerMimeType == "video/h264; profile-level-id=644016") { // L2.2
        maxSize = KResH264_PLID_428016;
    } else if (lowerMimeType == "video/h264; profile-level-id=42801e" ||
               lowerMimeType == "video/h264; profile-level-id=4d401e" ||
               lowerMimeType == "video/h264; profile-level-id=64401e") { // L3
        maxSize = KResH264_PLID_42801E;
    } else if (lowerMimeType == "video/h264; profile-level-id=42801f" ||
               lowerMimeType == "video/h264; profile-level-id=4d401f" ||
               lowerMimeType == "video/h264; profile-level-id=64401f") { // L3.1
        maxSize = KResH264_PLID_42801F;
    } else if (lowerMimeType == "video/h264; profile-level-id=428020" ||
               lowerMimeType == "video/h264; profile-level-id=4d4020" ||
               lowerMimeType == "video/h264; profile-level-id=644020") { // L3.2
        maxSize = KResH264_PLID_428020;
    } else if (lowerMimeType == "video/h264; profile-level-id=428028" ||
               lowerMimeType == "video/h264; profile-level-id=4d4028" ||
               lowerMimeType == "video/h264; profile-level-id=644028") { // L4
        maxSize = KResH264_PLID_428028;
    }

    return maxSize;
}

/*
 * This function returns the maximum framerate defined by the video standards
 * for different MIME Types.
 */

qreal S60VideoCaptureSession::maximumFrameRateForMimeType(const QString &mimeType) const
{
    qreal maxRate(-1.0);
    // Use all lower case for comparisons
    QString lowerMimeType = mimeType.toLower();

    if (lowerMimeType == "video/h263-2000") {
        maxRate = KFrR_H263;
    } else if (lowerMimeType == "video/h263-2000; profile=0") {
        maxRate = KFrR_H263_Profile0;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=10") {
        maxRate = KFrR_H263_Profile0_Level10;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=20") {
        maxRate = KFrR_H263_Profile0_Level20;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=30") {
        maxRate = KFrR_H263_Profile0_Level30;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=40") {
        maxRate = KFrR_H263_Profile0_Level40;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=45") {
        maxRate = KFrR_H263_Profile0_Level45;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=50") {
        maxRate = KFrR_H263_Profile0_Level50;
    } else if (lowerMimeType == "video/h263-2000; profile=3") {
        maxRate = KFrR_H263_Profile3;
    } else if (lowerMimeType == "video/mp4v-es") {
        maxRate = KFrR_MPEG4;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=1") {
        maxRate = KFrR_MPEG4_PLID_1;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=2") {
        maxRate = KFrR_MPEG4_PLID_2;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=3") {
        maxRate = KFrR_MPEG4_PLID_3;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=4") {
        maxRate = KFrR_MPEG4_PLID_4;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=5") {
        maxRate = KFrR_MPEG4_PLID_5;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=6") {
        maxRate = KFrR_MPEG4_PLID_6;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=8") {
        maxRate = KFrR_MPEG4_PLID_8;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=9") {
        maxRate = KFrR_MPEG4_PLID_9;
    } else if (lowerMimeType == "video/h264") {
        maxRate = KFrR_H264;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800a" ||
               lowerMimeType == "video/h264; profile-level-id=4d400a" ||
               lowerMimeType == "video/h264; profile-level-id=64400a") { // L1
        maxRate = KFrR_H264_PLID_42800A;
    } else if (lowerMimeType == "video/h264; profile-level-id=42900b" ||
               lowerMimeType == "video/h264; profile-level-id=4d500b" ||
               lowerMimeType == "video/h264; profile-level-id=644009") { // L1.b
        maxRate = KFrR_H264_PLID_42900B;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800b" ||
               lowerMimeType == "video/h264; profile-level-id=4d400b" ||
               lowerMimeType == "video/h264; profile-level-id=64400b") { // L1.1
        maxRate = KFrR_H264_PLID_42800B;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800c" ||
               lowerMimeType == "video/h264; profile-level-id=4d400c" ||
               lowerMimeType == "video/h264; profile-level-id=64400c") { // L1.2
        maxRate = KFrR_H264_PLID_42800C;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800d" ||
               lowerMimeType == "video/h264; profile-level-id=4d400d" ||
               lowerMimeType == "video/h264; profile-level-id=64400d") { // L1.3
        maxRate = KFrR_H264_PLID_42800D;
    } else if (lowerMimeType == "video/h264; profile-level-id=428014" ||
               lowerMimeType == "video/h264; profile-level-id=4d4014" ||
               lowerMimeType == "video/h264; profile-level-id=644014") { // L2
        maxRate = KFrR_H264_PLID_428014;
    } else if (lowerMimeType == "video/h264; profile-level-id=428015" ||
               lowerMimeType == "video/h264; profile-level-id=4d4015" ||
               lowerMimeType == "video/h264; profile-level-id=644015") { // L2.1
        maxRate = KFrR_H264_PLID_428015;
    } else if (lowerMimeType == "video/h264; profile-level-id=428016" ||
               lowerMimeType == "video/h264; profile-level-id=4d4016" ||
               lowerMimeType == "video/h264; profile-level-id=644016") { // L2.2
        maxRate = KFrR_H264_PLID_428016;
    } else if (lowerMimeType == "video/h264; profile-level-id=42801e" ||
               lowerMimeType == "video/h264; profile-level-id=4d401e" ||
               lowerMimeType == "video/h264; profile-level-id=64401e") { // L3
        maxRate = KFrR_H264_PLID_42801E;
    } else if (lowerMimeType == "video/h264; profile-level-id=42801f" ||
               lowerMimeType == "video/h264; profile-level-id=4d401f" ||
               lowerMimeType == "video/h264; profile-level-id=64401f") { // L3.1
        maxRate = KFrR_H264_PLID_42801F;
    } else if (lowerMimeType == "video/h264; profile-level-id=428020" ||
               lowerMimeType == "video/h264; profile-level-id=4d4020" ||
               lowerMimeType == "video/h264; profile-level-id=644020") { // L3.2
        maxRate = KFrR_H264_PLID_428020;
    } else if (lowerMimeType == "video/h264; profile-level-id=428028" ||
               lowerMimeType == "video/h264; profile-level-id=4d4028" ||
               lowerMimeType == "video/h264; profile-level-id=644028") { // L4
        maxRate = KFrR_H264_PLID_428028;
    }

    return maxRate;
}

/*
 * This function returns the maximum bitrate defined by the video standards
 * for different MIME Types.
 */
int S60VideoCaptureSession::maximumBitRateForMimeType(const QString &mimeType) const
{
    int maxRate(-1.0);
    // Use all lower case for comparisons
    QString lowerMimeType = mimeType.toLower();

    if (lowerMimeType == "video/h263-2000") {
        maxRate = KBiR_H263;
    } else if (lowerMimeType == "video/h263-2000; profile=0") {
        maxRate = KBiR_H263_Profile0;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=10") {
        maxRate = KBiR_H263_Profile0_Level10;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=20") {
        maxRate = KBiR_H263_Profile0_Level20;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=30") {
        maxRate = KBiR_H263_Profile0_Level30;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=40") {
        maxRate = KBiR_H263_Profile0_Level40;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=45") {
        maxRate = KBiR_H263_Profile0_Level45;
    } else if (lowerMimeType == "video/h263-2000; profile=0; level=50") {
        maxRate = KBiR_H263_Profile0_Level50;
    } else if (lowerMimeType == "video/h263-2000; profile=3") {
        maxRate = KBiR_H263_Profile3;
    } else if (lowerMimeType == "video/mp4v-es") {
        maxRate = KBiR_MPEG4;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=1") {
        maxRate = KBiR_MPEG4_PLID_1;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=2") {
        maxRate = KBiR_MPEG4_PLID_2;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=3") {
        maxRate = KBiR_MPEG4_PLID_3;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=4") {
        maxRate = KBiR_MPEG4_PLID_4;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=5") {
        maxRate = KBiR_MPEG4_PLID_5;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=6") {
        maxRate = KBiR_MPEG4_PLID_6;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=8") {
        maxRate = KBiR_MPEG4_PLID_8;
    } else if (lowerMimeType == "video/mp4v-es; profile-level-id=9") {
        maxRate = KBiR_MPEG4_PLID_9;
    } else if (lowerMimeType == "video/h264") {
        maxRate = KBiR_H264;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800a" ||
               lowerMimeType == "video/h264; profile-level-id=4d400a" ||
               lowerMimeType == "video/h264; profile-level-id=64400a") { // L1
        maxRate = KBiR_H264_PLID_42800A;
    } else if (lowerMimeType == "video/h264; profile-level-id=42900b" ||
               lowerMimeType == "video/h264; profile-level-id=4d500b" ||
               lowerMimeType == "video/h264; profile-level-id=644009") { // L1.b
        maxRate = KBiR_H264_PLID_42900B;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800b" ||
               lowerMimeType == "video/h264; profile-level-id=4d400b" ||
               lowerMimeType == "video/h264; profile-level-id=64400b") { // L1.1
        maxRate = KBiR_H264_PLID_42800B;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800c" ||
               lowerMimeType == "video/h264; profile-level-id=4d400c" ||
               lowerMimeType == "video/h264; profile-level-id=64400c") { // L1.2
        maxRate = KBiR_H264_PLID_42800C;
    } else if (lowerMimeType == "video/h264; profile-level-id=42800d" ||
               lowerMimeType == "video/h264; profile-level-id=4d400d" ||
               lowerMimeType == "video/h264; profile-level-id=64400d") { // L1.3
        maxRate = KBiR_H264_PLID_42800D;
    } else if (lowerMimeType == "video/h264; profile-level-id=428014" ||
               lowerMimeType == "video/h264; profile-level-id=4d4014" ||
               lowerMimeType == "video/h264; profile-level-id=644014") { // L2
        maxRate = KBiR_H264_PLID_428014;
    } else if (lowerMimeType == "video/h264; profile-level-id=428015" ||
               lowerMimeType == "video/h264; profile-level-id=4d4015" ||
               lowerMimeType == "video/h264; profile-level-id=644015") { // L2.1
        maxRate = KBiR_H264_PLID_428015;
    } else if (lowerMimeType == "video/h264; profile-level-id=428016" ||
               lowerMimeType == "video/h264; profile-level-id=4d4016" ||
               lowerMimeType == "video/h264; profile-level-id=644016") { // L2.2
        maxRate = KBiR_H264_PLID_428016;
    } else if (lowerMimeType == "video/h264; profile-level-id=42801e" ||
               lowerMimeType == "video/h264; profile-level-id=4d401e" ||
               lowerMimeType == "video/h264; profile-level-id=64401e") { // L3
        maxRate = KBiR_H264_PLID_42801E;
    } else if (lowerMimeType == "video/h264; profile-level-id=42801f" ||
               lowerMimeType == "video/h264; profile-level-id=4d401f" ||
               lowerMimeType == "video/h264; profile-level-id=64401f") { // L3.1
        maxRate = KBiR_H264_PLID_42801F;
    } else if (lowerMimeType == "video/h264; profile-level-id=428020" ||
               lowerMimeType == "video/h264; profile-level-id=4d4020" ||
               lowerMimeType == "video/h264; profile-level-id=644020") { // L3.2
        maxRate = KBiR_H264_PLID_428020;
    } else if (lowerMimeType == "video/h264; profile-level-id=428028" ||
               lowerMimeType == "video/h264; profile-level-id=4d4028" ||
               lowerMimeType == "video/h264; profile-level-id=644028") { // L4
        maxRate = KBiR_H264_PLID_428028;
    }

    return maxRate;
}

// End of file
