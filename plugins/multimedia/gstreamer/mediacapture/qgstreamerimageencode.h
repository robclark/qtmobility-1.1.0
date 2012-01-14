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

#ifndef QGSTREAMERIMAGEENCODE_H
#define QGSTREAMERIMAGEENCODE_H

class QGstreamerCaptureSession;

#include <qimageencodercontrol.h>

#include <QtCore/qstringlist.h>
#include <QtCore/qmap.h>

#include <gst/gst.h>
QT_USE_NAMESPACE

class QGstreamerImageEncode : public QImageEncoderControl
{
    Q_OBJECT
public:
    QGstreamerImageEncode(QGstreamerCaptureSession *session);
    virtual ~QGstreamerImageEncode();

    QList<QSize> supportedResolutions(const QImageEncoderSettings &settings = QImageEncoderSettings(),
                                      bool *continuous = 0) const;

    QStringList supportedImageCodecs() const;
    QString imageCodecDescription(const QString &codecName) const;

    QImageEncoderSettings imageSettings() const;
    void setImageSettings(const QImageEncoderSettings &settings);

Q_SIGNALS:
    void settingsChanged();

private:
    QImageEncoderSettings m_settings;

    QGstreamerCaptureSession *m_session;
};

#endif