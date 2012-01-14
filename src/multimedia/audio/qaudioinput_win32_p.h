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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QAUDIOINPUTWIN_H
#define QAUDIOINPUTWIN_H

#include <windows.h>
#include <mmsystem.h>

#include <QtCore/qfile.h>
#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmutex.h>

#include <qaudio.h>
#include <qaudiodeviceinfo.h>
#include <qaudiosystem.h>


QT_BEGIN_NAMESPACE

// For compat with 4.6
#if !defined(QT_WIN_CALLBACK)
#  if defined(Q_CC_MINGW)
#    define QT_WIN_CALLBACK CALLBACK __attribute__ ((force_align_arg_pointer))
#  else
#    define QT_WIN_CALLBACK CALLBACK
#  endif
#endif

class QAudioInputPrivate : public QAbstractAudioInput
{
    Q_OBJECT
public:
    QAudioInputPrivate(const QByteArray &device);
    ~QAudioInputPrivate();

    qint64 read(char* data, qint64 len);

    void setFormat(const QAudioFormat& fmt);
    QAudioFormat format() const;
    QIODevice* start();
    void start(QIODevice* device);
    void stop();
    void reset();
    void suspend();
    void resume();
    int bytesReady() const;
    int periodSize() const;
    void setBufferSize(int value);
    int bufferSize() const;
    void setNotifyInterval(int milliSeconds);
    int notifyInterval() const;
    qint64 processedUSecs() const;
    qint64 elapsedUSecs() const;
    QAudio::Error error() const;
    QAudio::State state() const;

    QIODevice* audioSource;
    QAudioFormat settings;
    QAudio::Error errorState;
    QAudio::State deviceState;

private:
    qint32 buffer_size;
    qint32 period_size;
    qint32 header;
    QByteArray m_device;
    int bytesAvailable;
    int intervalTime;
    QTime timeStamp;
    qint64 elapsedTimeOffset;
    QTime timeStampOpened;
    qint64 totalTimeValue;
    bool pullMode;
    bool resuming;
    WAVEFORMATEX wfx;
    HWAVEIN hWaveIn;
    MMRESULT result;
    WAVEHDR* waveBlocks;
    volatile bool finished;
    volatile int waveFreeBlockCount;
    int waveCurrentBlock;

    QMutex mutex;
    static void QT_WIN_CALLBACK waveInProc( HWAVEIN hWaveIn, UINT uMsg,
            DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 );

    WAVEHDR* allocateBlocks(int size, int count);
    void freeBlocks(WAVEHDR* blockArray);
    bool open();
    void close();

private slots:
    void feedback();
    bool deviceReady();

signals:
    void processMore();
};

class InputPrivate : public QIODevice
{
    Q_OBJECT
public:
    InputPrivate(QAudioInputPrivate* audio);
    ~InputPrivate();

    qint64 readData( char* data, qint64 len);
    qint64 writeData(const char* data, qint64 len);

    void trigger();
private:
    QAudioInputPrivate *audioDevice;
};

QT_END_NAMESPACE

#endif
