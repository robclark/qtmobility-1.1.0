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

#ifndef QDECLARATIVECONTACTRELATIONSHIPFILTER_H
#define QDECLARATIVECONTACTRELATIONSHIPFILTER_H

#include "qdeclarativecontactfilter_p.h"
#include "qcontactrelationshipfilter.h"
#include "qdeclarativecontactrelationship_p.h"

class QDeclarativeContactRelationshipFilter : public QDeclarativeContactFilter
{
    Q_OBJECT
    Q_PROPERTY(QVariant relationshipType READ relationshipType WRITE setRelationshipType NOTIFY valueChanged)
    Q_PROPERTY(QContactLocalId relatedContactId READ relatedContactId WRITE setRelatedContactId NOTIFY valueChanged)
    Q_PROPERTY(QDeclarativeContactRelationship::RelationshipRole relatedContactRole READ relatedContactRole WRITE setRelatedContactRole NOTIFY valueChanged)

public:

    QDeclarativeContactRelationshipFilter(QObject* parent = 0)
        :QDeclarativeContactFilter(parent)
    {
    }

    QVariant relationshipType() const
    {
       return r.relationshipType();
    }
    void setRelationshipType(const QVariant& v)
    {
        if (v != relationshipType()) {
            r.setRelationshipType(v);
            emit valueChanged();
        }
    }

    QContactLocalId relatedContactId() const
    {
        return d.relatedContactId().localId();
    }

    void setRelatedContactId(const QContactLocalId& v)
    {
        if (v != relatedContactId()) {
            QContactId contactId;
            contactId.setLocalId(v);
            d.setRelatedContactId(contactId);
            emit valueChanged();
        }
    }

    QDeclarativeContactRelationship::RelationshipRole relatedContactRole() const
    {
        switch (d.relatedContactRole()) {
        case QContactRelationship::First:
            return QDeclarativeContactRelationship::First;
        case QContactRelationship::Second:
            return QDeclarativeContactRelationship::Second;
        default:
            break;
        }
        return QDeclarativeContactRelationship::Either;
    }

    void setRelatedContactRole(QDeclarativeContactRelationship::RelationshipRole v)
    {
        if (v != relatedContactRole()) {
            switch (v) {
            case QDeclarativeContactRelationship::First:
                d.setRelatedContactRole(QContactRelationship::First);
            case QDeclarativeContactRelationship::Second:
                d.setRelatedContactRole(QContactRelationship::Second);
            case QDeclarativeContactRelationship::Either:
                break;
            }
            d.setRelatedContactRole(QContactRelationship::Either);
            emit valueChanged();
        }
    }
    QContactFilter filter() const
    {
        QContactRelationshipFilter filter(d);
        filter.setRelationshipType(r.relationship().relationshipType());
        return filter;
    }
signals:
    void valueChanged();


private:
    QDeclarativeContactRelationship r;
    QContactRelationshipFilter d;
};

QML_DECLARE_TYPE(QDeclarativeContactRelationshipFilter)

#endif
