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

#ifndef V4LRADIOCONTROL_H
#define V4LRADIOCONTROL_H

#include <QtCore/qobject.h>
#include <QtCore/qtimer.h>

#include <qradiotunercontrol.h>

#include <QtDBus/QtDBus>
#include <gst/gst.h>

#include <alsa/asoundlib.h>

QT_USE_NAMESPACE

class V4LRadioService;

class V4LRadioControl : public QRadioTunerControl
{
    Q_OBJECT
public:
    V4LRadioControl(QObject *parent = 0);
    ~V4LRadioControl();

    bool isAvailable() const;
    QtMultimediaKit::AvailabilityError availabilityError() const;

    QRadioTuner::State state() const;

    QRadioTuner::Band band() const;
    void setBand(QRadioTuner::Band b);
    bool isBandSupported(QRadioTuner::Band b) const;

    int frequency() const;
    int frequencyStep(QRadioTuner::Band b) const;
    QPair<int,int> frequencyRange(QRadioTuner::Band b) const;
    void setFrequency(int frequency);

    bool isStereo() const;
    QRadioTuner::StereoMode stereoMode() const;
    void setStereoMode(QRadioTuner::StereoMode mode);

    int signalStrength() const;

    int volume() const;
    void setVolume(int volume);

    bool isMuted() const;
    void setMuted(bool muted);

    bool isSearching() const;
    void cancelSearch();

    void searchForward();
    void searchBackward();

    void start();
    void stop();

    QRadioTuner::Error error() const;
    QString errorString() const;

public slots:
    void enablePipeline(bool enable = true);

private slots:
    void search();
    void enableFMRX();

private:
    bool initRadio();
    void setupHeadPhone();
    bool createGstPipeline();
    void callAmixer(const QString& target, const QString& value);
    int getEnumItemIndex(snd_ctl_t *handle, snd_ctl_elem_info_t *info, const  QString &value);
    int vol(snd_hctl_elem_t *elem) const;

private:
    int fd;

    bool m_error;
    bool muted;
    bool stereo;
    bool low;
    bool available;
    int  tuners;
    int  step;
    int  sig;
    bool scanning;
    bool forward;
    QTimer* timer;
    QTimer* tickTimer;
    QRadioTuner::Band   currentBand;
    qint64 freqMin;
    qint64 freqMax;
    qint64 currentFreq;

    GstElement *pipeline;

    QDBusInterface* FMRXEnablerIFace;
};

#endif
