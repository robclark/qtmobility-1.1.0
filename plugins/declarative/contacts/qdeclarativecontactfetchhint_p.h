/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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
#ifndef QDECLARATIVECONTACTFETCHHINT_H
#define QDECLARATIVECONTACTFETCHHINT_H

#include <QVariant>
#include <QList>
#include <QSize>

#include <qdeclarative.h>
#include <QDeclarativeExtensionPlugin>

#include "qcontactfetchhint.h"

QTM_USE_NAMESPACE

class QDeclarativeContactFetchHint : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList detailDefinitionsHint READ detailDefinitionsHint WRITE setDetailDefinitionsHint NOTIFY fetchHintChanged )
    Q_PROPERTY(QStringList relationshipTypesHint READ relationshipTypesHint WRITE setRelationshipTypesHint NOTIFY fetchHintChanged )
    Q_PROPERTY(OptimizationHints optimizationHints READ optimizationHints WRITE setOptimizationHints NOTIFY fetchHintChanged )
    Q_PROPERTY(QSize optimizationHints READ preferredImageSize WRITE setPreferredImageSize NOTIFY fetchHintChanged )
    Q_FLAGS(OptimizationHints);
public:
    enum OptimizationHint {
        AllRequired = QContactFetchHint::AllRequired,
        NoRelationships = QContactFetchHint::NoRelationships,
        NoActionPreferences = QContactFetchHint::NoActionPreferences,
        NoBinaryBlobs = QContactFetchHint::NoBinaryBlobs
    };
    Q_DECLARE_FLAGS(OptimizationHints, OptimizationHint)

    QDeclarativeContactFetchHint(QObject* parent = 0);
    QStringList detailDefinitionsHint() const;
    void setDetailDefinitionsHint(const QStringList& definitionNames);

    QStringList relationshipTypesHint() const;
    void setRelationshipTypesHint(const QStringList& relationshipTypes);

    QSize preferredImageSize() const;
    void setPreferredImageSize(const QSize& size);


    OptimizationHints optimizationHints() const;
    void setOptimizationHints(OptimizationHints hints);

    QContactFetchHint fetchHint() const;
    void setFetchHint(const QContactFetchHint& fetchHint);
signals:
    void fetchHintChanged();
private:
    QContactFetchHint m_fetchHint;
};

QML_DECLARE_TYPE(QDeclarativeContactFetchHint)

#endif
