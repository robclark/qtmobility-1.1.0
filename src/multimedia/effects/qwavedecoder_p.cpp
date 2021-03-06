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

#include "qwavedecoder_p.h"

#include <QtCore/qtimer.h>
#include <QtCore/qendian.h>

QT_BEGIN_NAMESPACE

QWaveDecoder::QWaveDecoder(QIODevice *s, QObject *parent):
    QIODevice(parent),
    haveFormat(false),
    dataSize(0),
    remaining(0),
    source(s)
{
    open(QIODevice::ReadOnly | QIODevice::Unbuffered);

    if (source->bytesAvailable() >= qint64(sizeof(CombinedHeader) + sizeof(DATAHeader) + sizeof(quint16)))
        QTimer::singleShot(0, this, SLOT(handleData()));
    else
        connect(source, SIGNAL(readyRead()), SLOT(handleData()));
}

QWaveDecoder::~QWaveDecoder()
{
}

QAudioFormat QWaveDecoder::audioFormat() const
{
    return format;
}

int QWaveDecoder::duration() const
{
    return size() * 1000 / (format.sampleSize() / 8) / format.channels() / format.frequency();
}

qint64 QWaveDecoder::size() const
{
    return haveFormat ? dataSize : 0;
}

bool QWaveDecoder::isSequential() const
{
    return source->isSequential();
}

qint64 QWaveDecoder::bytesAvailable() const
{
    return haveFormat ? source->bytesAvailable() : 0;
}

qint64 QWaveDecoder::readData(char *data, qint64 maxlen)
{
    return haveFormat ? source->read(data, maxlen) : 0;
}

qint64 QWaveDecoder::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return -1;
}

void QWaveDecoder::handleData()
{
    if (source->bytesAvailable() < qint64(sizeof(CombinedHeader) + sizeof(DATAHeader) + sizeof(quint16)))
        return;

    source->disconnect(SIGNAL(readyRead()), this, SLOT(handleData()));
    source->read((char*)&header, sizeof(CombinedHeader));

    if (qstrncmp(header.riff.descriptor.id, "RIFF", 4) != 0 ||
        qstrncmp(header.riff.type, "WAVE", 4) != 0 ||
        qstrncmp(header.wave.descriptor.id, "fmt ", 4) != 0 ||
        (header.wave.audioFormat != 0 && header.wave.audioFormat != 1)) {

        emit invalidFormat();
    }
    else {
        DATAHeader dataHeader;

        if (qFromLittleEndian<quint32>(header.wave.descriptor.size) > sizeof(WAVEHeader)) {
            // Extended data available
            quint16 extraFormatBytes;
            source->peek((char*)&extraFormatBytes, sizeof(quint16));
            extraFormatBytes = qFromLittleEndian<quint16>(extraFormatBytes);
            source->read(sizeof(quint16) + extraFormatBytes);   // dump it all
        }

        source->read((char*)&dataHeader, sizeof(DATAHeader));

        int bps = qFromLittleEndian<quint16>(header.wave.bitsPerSample);

        format.setCodec(QLatin1String("audio/pcm"));
        format.setSampleType(bps == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setFrequency(qFromLittleEndian<quint32>(header.wave.sampleRate));
        format.setSampleSize(bps);
        format.setChannels(qFromLittleEndian<quint16>(header.wave.numChannels));

        dataSize = qFromLittleEndian<quint32>(dataHeader.descriptor.size);

        haveFormat = true;
        connect(source, SIGNAL(readyRead()), SIGNAL(readyRead()));
        emit formatKnown();
    }
}

QT_END_NAMESPACE

#include "moc_qwavedecoder_p.cpp"
