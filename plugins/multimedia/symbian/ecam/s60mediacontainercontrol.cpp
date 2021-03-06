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

#include "s60mediacontainercontrol.h"
#include "s60videocapturesession.h"
#include "s60cameraconstants.h"

S60MediaContainerControl::S60MediaContainerControl(QObject *parent):
    QMediaContainerControl(parent)
{
}

S60MediaContainerControl::S60MediaContainerControl(S60VideoCaptureSession *session, QObject *parent):
    QMediaContainerControl(parent)
{
    if (session)
        m_session = session;
    else
        Q_ASSERT(true);
    // From now on it is safe to assume session exists

    // Set default video container
    m_supportedContainers = m_session->supportedVideoContainers();

    if (!m_supportedContainers.isEmpty()) {
        // Check if default container is supported
        if (m_supportedContainers.indexOf(KMimeTypeDefaultContainer) != -1)
            setContainerMimeType(KMimeTypeDefaultContainer);
        // Otherwise use first in the list
        else
            setContainerMimeType(m_supportedContainers[0]); // First as default
    } else {
        m_session->setError(KErrGeneral, QString("No supported video containers found."));
    }
}

S60MediaContainerControl::~S60MediaContainerControl()
{
    m_supportedContainers.clear();
    m_containerDescriptions.clear();
}

QStringList S60MediaContainerControl::supportedContainers() const
{
    return m_session->supportedVideoContainers();
}

QString S60MediaContainerControl::containerMimeType() const
{
    return m_session->videoContainer();
}

void S60MediaContainerControl::setContainerMimeType(const QString &containerMimeType)
{
    m_session->setVideoContainer(containerMimeType);
}

QString S60MediaContainerControl::containerDescription(const QString &containerMimeType) const
{
    return m_session->videoContainerDescription(containerMimeType);
}

// End of file
