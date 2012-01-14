/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
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


#include <qaudiodeviceinfo.h>

#include "audiodevices.h"

// Utility functions for converting QAudioFormat fields into text

QString toString(QAudioFormat::SampleType sampleType)
{
    QString result("Unknown");
    switch (sampleType) {
    case QAudioFormat::SignedInt:
        result = "SignedInt";
        break;
    case QAudioFormat::UnSignedInt:
        result = "UnSignedInt";
        break;
    case QAudioFormat::Float:
        result = "Float";
        break;
    }
    return result;
}

QString toString(QAudioFormat::Endian endian)
{
    QString result("Unknown");
    switch (endian) {
    case QAudioFormat::LittleEndian:
        result = "LittleEndian";
        break;
    case QAudioFormat::BigEndian:
        result = "BigEndian";
        break;
    }
    return result;
}


AudioDevicesBase::AudioDevicesBase(QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f)
{
    setupUi(this);
}

AudioDevicesBase::~AudioDevicesBase() {}


AudioTest::AudioTest(QWidget *parent, Qt::WFlags f)
    : AudioDevicesBase(parent, f)
{
    mode = QAudio::AudioOutput;

    connect(testButton, SIGNAL(clicked()), SLOT(test()));
    connect(modeBox, SIGNAL(activated(int)), SLOT(modeChanged(int)));
    connect(deviceBox, SIGNAL(activated(int)), SLOT(deviceChanged(int)));
    connect(frequencyBox, SIGNAL(activated(int)), SLOT(freqChanged(int)));
    connect(channelsBox, SIGNAL(activated(int)), SLOT(channelChanged(int)));
    connect(codecsBox, SIGNAL(activated(int)), SLOT(codecChanged(int)));
    connect(sampleSizesBox, SIGNAL(activated(int)), SLOT(sampleSizeChanged(int)));
    connect(sampleTypesBox, SIGNAL(activated(int)), SLOT(sampleTypeChanged(int)));
    connect(endianBox, SIGNAL(activated(int)), SLOT(endianChanged(int)));
    connect(populateTableButton, SIGNAL(clicked()), SLOT(populateTable()));

    modeBox->setCurrentIndex(0);
    modeChanged(0);
    deviceBox->setCurrentIndex(0);
    deviceChanged(0);
}

AudioTest::~AudioTest()
{
}

void AudioTest::test()
{
    // tries to set all the settings picked.
    testResult->clear();

    if (!deviceInfo.isNull()) {
        if (deviceInfo.isFormatSupported(settings)) {
            testResult->setText(tr("Success"));
            nearestFreq->setText("");
            nearestChannel->setText("");
            nearestCodec->setText("");
            nearestSampleSize->setText("");
            nearestSampleType->setText("");
            nearestEndian->setText("");
        } else {
            QAudioFormat nearest = deviceInfo.nearestFormat(settings);
            testResult->setText(tr("Failed"));
            nearestFreq->setText(QString("%1").arg(nearest.frequency()));
            nearestChannel->setText(QString("%1").arg(nearest.channels()));
            nearestCodec->setText(nearest.codec());
            nearestSampleSize->setText(QString("%1").arg(nearest.sampleSize()));
            nearestSampleType->setText(toString(nearest.sampleType()));
            nearestEndian->setText(toString(nearest.byteOrder()));
        }
    }
    else
        testResult->setText(tr("No Device"));
}

void AudioTest::modeChanged(int idx)
{
    testResult->clear();

    // mode has changed
    if (idx == 0)
        mode = QAudio::AudioInput;
    else
        mode = QAudio::AudioOutput;

    deviceBox->clear();
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(mode))
        deviceBox->addItem(deviceInfo.deviceName(), qVariantFromValue(deviceInfo));

    deviceBox->setCurrentIndex(0);
    deviceChanged(0);
}

void AudioTest::deviceChanged(int idx)
{
    testResult->clear();

    if (deviceBox->count() == 0)
        return;

    // device has changed
    deviceInfo = deviceBox->itemData(idx).value<QAudioDeviceInfo>();

    frequencyBox->clear();
    QList<int> freqz = deviceInfo.supportedFrequencies();
    for(int i = 0; i < freqz.size(); ++i)
        frequencyBox->addItem(QString("%1").arg(freqz.at(i)));
    if(freqz.size())
        settings.setFrequency(freqz.at(0));

    channelsBox->clear();
    QList<int> chz = deviceInfo.supportedChannels();
    for(int i = 0; i < chz.size(); ++i)
        channelsBox->addItem(QString("%1").arg(chz.at(i)));
    if(chz.size())
        settings.setChannels(chz.at(0));

    codecsBox->clear();
    QStringList codecz = deviceInfo.supportedCodecs();
    for (int i = 0; i < codecz.size(); ++i)
        codecsBox->addItem(QString("%1").arg(codecz.at(i)));
    if (codecz.size())
        settings.setCodec(codecz.at(0));
    // Add false to create failed condition!
    codecsBox->addItem("audio/test");

    sampleSizesBox->clear();
    QList<int> sampleSizez = deviceInfo.supportedSampleSizes();
    for (int i = 0; i < sampleSizez.size(); ++i)
        sampleSizesBox->addItem(QString("%1").arg(sampleSizez.at(i)));
    if (sampleSizez.size())
        settings.setSampleSize(sampleSizez.at(0));

    sampleTypesBox->clear();
    QList<QAudioFormat::SampleType> sampleTypez = deviceInfo.supportedSampleTypes();

    for (int i = 0; i < sampleTypez.size(); ++i)
        sampleTypesBox->addItem(toString(sampleTypez.at(i)));
    if (sampleTypez.size())
        settings.setSampleType(sampleTypez.at(0));

    endianBox->clear();
    QList<QAudioFormat::Endian> endianz = deviceInfo.supportedByteOrders();
    for (int i = 0; i < endianz.size(); ++i)
        endianBox->addItem(toString(endianz.at(i)));
    if (endianz.size())
        settings.setByteOrder(endianz.at(0));

    allFormatsTable->clearContents();
}

void AudioTest::populateTable()
{
    int row = 0;

    QAudioFormat format;
    foreach (QString codec, deviceInfo.supportedCodecs()) {
        format.setCodec(codec);
        foreach (int frequency, deviceInfo.supportedFrequencies()) {
            format.setFrequency(frequency);
            foreach (int channels, deviceInfo.supportedChannels()) {
                format.setChannels(channels);
                foreach (QAudioFormat::SampleType sampleType, deviceInfo.supportedSampleTypes()) {
                    format.setSampleType(sampleType);
                    foreach (int sampleSize, deviceInfo.supportedSampleSizes()) {
                        format.setSampleSize(sampleSize);
                        foreach (QAudioFormat::Endian endian, deviceInfo.supportedByteOrders()) {
                            format.setByteOrder(endian);
                            if (deviceInfo.isFormatSupported(format)) {
                                allFormatsTable->setRowCount(row + 1);

                                QTableWidgetItem *codecItem = new QTableWidgetItem(format.codec());
                                allFormatsTable->setItem(row, 0, codecItem);

                                QTableWidgetItem *frequencyItem = new QTableWidgetItem(QString("%1").arg(format.frequency()));
                                allFormatsTable->setItem(row, 1, frequencyItem);

                                QTableWidgetItem *channelsItem = new QTableWidgetItem(QString("%1").arg(format.channels()));
                                allFormatsTable->setItem(row, 2, channelsItem);

                                QTableWidgetItem *sampleTypeItem = new QTableWidgetItem(toString(format.sampleType()));
                                allFormatsTable->setItem(row, 3, sampleTypeItem);

                                QTableWidgetItem *sampleSizeItem = new QTableWidgetItem(QString("%1").arg(format.sampleSize()));
                                allFormatsTable->setItem(row, 4, sampleSizeItem);

                                QTableWidgetItem *byteOrderItem = new QTableWidgetItem(toString(format.byteOrder()));
                                allFormatsTable->setItem(row, 5, byteOrderItem);

                                ++row;
                            }
                        }
                    }
                }
            }
        }
    }
}

void AudioTest::freqChanged(int idx)
{
    // freq has changed
    settings.setFrequency(frequencyBox->itemText(idx).toInt());
}

void AudioTest::channelChanged(int idx)
{
    settings.setChannels(channelsBox->itemText(idx).toInt());
}

void AudioTest::codecChanged(int idx)
{
    settings.setCodec(codecsBox->itemText(idx));
}

void AudioTest::sampleSizeChanged(int idx)
{
    settings.setSampleSize(sampleSizesBox->itemText(idx).toInt());
}

void AudioTest::sampleTypeChanged(int idx)
{
    switch (sampleTypesBox->itemText(idx).toInt()) {
        case QAudioFormat::SignedInt:
            settings.setSampleType(QAudioFormat::SignedInt);
            break;
        case QAudioFormat::UnSignedInt:
            settings.setSampleType(QAudioFormat::UnSignedInt);
            break;
        case QAudioFormat::Float:
            settings.setSampleType(QAudioFormat::Float);
    }
}

void AudioTest::endianChanged(int idx)
{
    switch (endianBox->itemText(idx).toInt()) {
        case QAudioFormat::LittleEndian:
            settings.setByteOrder(QAudioFormat::LittleEndian);
            break;
        case QAudioFormat::BigEndian:
            settings.setByteOrder(QAudioFormat::BigEndian);
    }
}
