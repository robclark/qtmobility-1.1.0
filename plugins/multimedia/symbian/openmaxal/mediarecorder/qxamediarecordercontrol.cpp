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

#include "qxamediarecordercontrol.h"
#include "qxarecordsession.h"
#include "qxacommon.h"

QXAMediaRecoderControl::QXAMediaRecoderControl(QXARecordSession *session, QObject *parent)
:QMediaRecorderControl(parent), m_session(session)
{
    connect(m_session, SIGNAL(stateChanged(QMediaRecorder::State)),
            this, SIGNAL(stateChanged(QMediaRecorder::State)));
    connect(m_session, SIGNAL(error(int,QString)),
        this,SIGNAL(error(int,QString)));
    connect(m_session, SIGNAL(durationChanged(qint64)),
        this, SIGNAL(durationChanged(qint64)));
}

QXAMediaRecoderControl::~QXAMediaRecoderControl()
{
    QT_TRACE_FUNCTION_ENTRY_EXIT;
}

QUrl QXAMediaRecoderControl::outputLocation() const
{
    if (m_session)
        return m_session->outputLocation();
    return QUrl();
}

bool QXAMediaRecoderControl::setOutputLocation(const QUrl &location)
{
    if (m_session)
        return m_session->setOutputLocation(location);
    return false;
}

QMediaRecorder::State QXAMediaRecoderControl::state() const
{
    if (m_session)
        return m_session->state();
    return QMediaRecorder::StoppedState;
}

qint64 QXAMediaRecoderControl::duration() const
{
    if (m_session)
        return m_session->duration();
    return 0;
}

void QXAMediaRecoderControl::record()
{
    if (m_session)
        m_session->record();
}

void QXAMediaRecoderControl::pause()
{
    if (m_session)
        m_session->pause();
}

void QXAMediaRecoderControl::stop()
{
    if (m_session)
        m_session->stop();
}

void QXAMediaRecoderControl::applySettings()
{
    if (m_session)
        m_session->applySettings();
}

bool QXAMediaRecoderControl::isMuted() const
{
    return false;
}

void QXAMediaRecoderControl::setMuted(bool)
{

}
