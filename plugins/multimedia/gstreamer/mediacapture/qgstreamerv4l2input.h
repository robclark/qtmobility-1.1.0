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


#ifndef QGSTREAMERV4L2INPUT_H
#define QGSTREAMERV4L2INPUT_H

#include <QtCore/qhash.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qsize.h>
#include "qgstreamercapturesession.h"

QT_USE_NAMESPACE

class QGstreamerV4L2Input : public QObject, public QGstreamerVideoInput
{
    Q_OBJECT
public:
    QGstreamerV4L2Input(QObject *parent = 0);
    virtual ~QGstreamerV4L2Input();

    GstElement *buildElement();

    QList<qreal> supportedFrameRates(const QSize &frameSize = QSize()) const;
    QList<QSize> supportedResolutions(qreal frameRate = -1) const;

    QByteArray device() const;

public slots:
    void setDevice(const QByteArray &device);
    void setDevice(const QString &device);

private:
    void updateSupportedResolutions(const QByteArray &device);

    QList<qreal> m_frameRates;
    QList<QSize> m_resolutions;

    QHash<QSize, QSet<int> > m_ratesByResolution;

    QByteArray m_device;
};

#endif // QGSTREAMERV4L2INPUT_H