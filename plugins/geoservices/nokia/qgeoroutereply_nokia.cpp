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
** This file is part of the Ovi services plugin for the Maps and
** Navigation API.  The use of these services, whether by use of the
** plugin or by other means, is governed by the terms and conditions
** described by the file OVI_SERVICES_TERMS_AND_CONDITIONS.txt in
** this package, located in the directory containing the Ovi services
** plugin source code.
**
****************************************************************************/

#include "qgeoroutereply_nokia.h"
#include "qgeoroutexmlparser.h"

#include <qgeorouterequest.h>

QGeoRouteReplyNokia::QGeoRouteReplyNokia(const QGeoRouteRequest &request, QNetworkReply *reply, QObject *parent)
        : QGeoRouteReply(request, parent),
        m_reply(reply)
{
    connect(m_reply,
            SIGNAL(finished()),
            this,
            SLOT(networkFinished()));

    connect(m_reply,
            SIGNAL(error(QNetworkReply::NetworkError)),
            this,
            SLOT(networkError(QNetworkReply::NetworkError)));
}

QGeoRouteReplyNokia::~QGeoRouteReplyNokia()
{
}

void QGeoRouteReplyNokia::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoRouteReplyNokia::networkFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError) {
        // Removed because this is already done in networkError, which previously caused _two_ errors to be raised for every error.
        //setError(QGeoRouteReply::CommunicationError, m_reply->errorString());
        //m_reply->deleteLater();
        //m_reply = 0;
        return;
    }

    QGeoRouteXmlParser parser(request());

    if (parser.parse(m_reply)) {
        setRoutes(parser.results());
        setFinished(true);
    } else {
        // add a qWarning with the actual parser.errorString()
        setError(QGeoRouteReply::ParseError, "The response from the service was not in a recognisable format.");
    }

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoRouteReplyNokia::networkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)

    if (!m_reply)
        return;

    setError(QGeoRouteReply::CommunicationError, m_reply->errorString());

    m_reply->deleteLater();
    m_reply = 0;
}
