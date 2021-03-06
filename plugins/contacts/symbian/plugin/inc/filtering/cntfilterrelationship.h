/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef CNTFILTERRELATIONSHIP_H_
#define CNTFILTERRELATIONSHIP_H_

#include "cntabstractcontactfilter.h"
#include "cntsymbiansrvconnection.h"
#include "cntdbinfo.h"

#include <qmobilityglobal.h>

QTM_USE_NAMESPACE

class CntFilterRelationship : public CntAbstractContactFilter
{
public:
    CntFilterRelationship(CContactDatabase& contactDatabase, CntSymbianSrvConnection &cntServer,CntDbInfo& dbInfo);
    ~CntFilterRelationship();
    QList<QContactLocalId> contacts(
            const QContactFilter &filter,
            const QList<QContactSortOrder> &sortOrders,
            bool &filterSupported,
            QContactManager::Error* error);
    bool filterSupported(const QContactFilter& filter) ;
    
    //bool isFilterSupported(const QContactFilter& filter) const;

    void CntFilterRelationship::getSqlquery( const QContactRelationshipFilter& relationfilter,
                                                   QString& sqlquery ,
                                                   QContactManager::Error* error) const;
    void CntFilterRelationship::createSelectQuery(const QContactFilter& detailFilter,
                                  QString& sqlQuery,
                                  QContactManager::Error* error);
protected:
    CContactDatabase& m_contactdatabase;
    CntSymbianSrvConnection &m_srvConnection;
    CntDbInfo& m_dbInfo;
};

#endif //CNTFILTERRELATIONSHIP_H_
