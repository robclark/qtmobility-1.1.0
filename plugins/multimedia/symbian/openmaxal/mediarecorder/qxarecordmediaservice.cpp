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

#include <QString>

#include "qxarecordmediaservice.h"
#include "qxarecordsession.h"
#include "qxamediarecordercontrol.h"
#include "qxaaudioendpointselector.h"
#include "qxaaudioencodercontrol.h"
#include "qxamediacontainercontrol.h"
#include "qxacommon.h"

QXARecodMediaService::QXARecodMediaService(QObject *parent)
:QMediaService(parent)
{
    QT_TRACE_FUNCTION_ENTRY;
    m_session = new QXARecordSession(this);
    m_control = new QXAMediaRecoderControl(m_session, this);
    m_endpoint = new QXAAudioEndpointSelector(m_session, this);
    m_encoder = new QXAAudioEncoderControl(m_session, this);
    m_container = new QXAMediaContainerControl(m_session, this);
}

QXARecodMediaService::~QXARecodMediaService()
{
    QT_TRACE_FUNCTION_ENTRY_EXIT;
}

QMediaControl* QXARecodMediaService::requestControl(const char *name)
{
    QT_TRACE_FUNCTION_ENTRY;
    if (qstrcmp(name, QMediaRecorderControl_iid) == 0)
        return m_control;
    else if (qstrcmp(name, QAudioEndpointSelector_iid) == 0)
        return m_endpoint;
    else if (qstrcmp(name, QAudioEncoderControl_iid) == 0)
        return m_encoder;
    else if (qstrcmp(name, QMediaContainerControl_iid) == 0)
        return m_container;
    QT_TRACE_FUNCTION_EXIT;
    return 0;
}

void QXARecodMediaService::releaseControl(QMediaControl *control)
{
    Q_UNUSED(control)
}