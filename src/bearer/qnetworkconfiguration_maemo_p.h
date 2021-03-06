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

#ifndef QNETWORKCONFIGURATIONPRIVATE_H
#define QNETWORKCONFIGURATIONPRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qshareddata.h>
#include <QNetworkInterface>

#include "qnetworkconfiguration.h"

QTM_BEGIN_NAMESPACE

class QNetworkConfigurationManagerPrivate;

class QNetworkConfigurationPrivate : public QSharedData
{
public:
    QNetworkConfigurationPrivate ()
	    : isValid(false), type(QNetworkConfiguration::Invalid), 
	    roamingSupported(false), purpose(QNetworkConfiguration::UnknownPurpose),
	    network_attrs(0), service_attrs(0), manager(0)
    {
    }

    ~QNetworkConfigurationPrivate()
    {
        //release pointers to member configurations
        serviceNetworkMembers.clear(); 
    }

    QString name;
    bool isValid;
    QString id;
    QNetworkConfiguration::StateFlags state;
    QNetworkConfiguration::Type type;
    bool roamingSupported;
    QNetworkConfiguration::Purpose purpose;

    QList<QExplicitlySharedDataPointer<QNetworkConfigurationPrivate> > serviceNetworkMembers;
    QNetworkInterface serviceInterface;

    /* In Maemo the id field (defined above) is the IAP id (which typically is UUID) */
    QByteArray network_id; /* typically WLAN ssid or similar */
    QString iap_type; /* is this one WLAN or GPRS */
    QString bearerName() const
    {
        if (iap_type == "WLAN_INFRA" ||
                iap_type == "WLAN_ADHOC")
            return QString("WLAN");
        else if (iap_type == "GPRS")
            return QString("HSPA");

        //return whatever it is 
        //this may have to be split up later on
        return iap_type;
    }

    quint32 network_attrs; /* network attributes for this IAP, this is the value returned by icd and passed to it when connecting */

    QString service_type;
    QString service_id;
    quint32 service_attrs;

    QNetworkConfigurationManagerPrivate *manager;

private:

    // disallow detaching
    QNetworkConfigurationPrivate &operator=(const QNetworkConfigurationPrivate &other);
    QNetworkConfigurationPrivate(const QNetworkConfigurationPrivate &other);
};

QTM_END_NAMESPACE

#endif //QNETWORKCONFIGURATIONPRIVATE_H
