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

#ifndef REQUESTEXAMPLE_H
#define REQUESTEXAMPLE_H

#include <QObject>
#include "qlandmarkfetchrequest.h"
#include "qlandmarkcategorysaverequest.h"
#include "qlandmarksaverequest.h"
#include "qlandmarkcategoryfetchrequest.h"
#include "qlandmarkcategoryremoverequest.h"
#include "qlandmarkremoverequest.h"
#include "qlandmarkimportrequest.h"
#include "qlandmarkexportrequest.h"
#include "qlandmarkmanager.h"


QTM_USE_NAMESPACE

class RequestExample : public QObject
{
    Q_OBJECT
public:
    RequestExample():QObject(),
                    lmManager(new QLandmarkManager(this)),
                    catSaveRequest(new QLandmarkCategorySaveRequest(lmManager, this)),
                    lmSaveRequest(new QLandmarkSaveRequest(lmManager, this)),
                    catFetchRequest(new QLandmarkCategoryFetchRequest(lmManager, this)),
                    lmFetchRequest(new QLandmarkFetchRequest(lmManager, this)),
                    catRemoveRequest(new QLandmarkCategoryRemoveRequest(lmManager, this)),
                    lmRemoveRequest(new QLandmarkRemoveRequest(lmManager, this))
                    {}
    ~RequestExample(){}

private slots:
    void categorySaveRequest();
    void categorySaveRequestHandler(QLandmarkAbstractRequest::State);
    void landmarkSaveRequest();
    void landmarkSaveRequestHandler(QLandmarkAbstractRequest::State);
    void categoryFetchRequest();
    void categoryFetchRequestHandler(QLandmarkAbstractRequest::State);
    void landmarkFetchRequest();
    void landmarkFetchRequestHandler(QLandmarkAbstractRequest::State);
    void categoryRemoveRequest();
    void categoryRemoveRequestHandler(QLandmarkAbstractRequest::State);
    void landmarkRemoveRequest();
    void landmarkRemoveRequestHandler(QLandmarkAbstractRequest::State);
    void landmarkImportRequest();
    void landmarkImportRequestHandler(QLandmarkAbstractRequest::State);
    void landmarkExportRequest();
    void landmarkExportRequestHandler(QLandmarkAbstractRequest::State);

private:
    QLandmarkManager *lmManager;
    QLandmarkFetchRequest *m_fetchRequest;
    QLandmarkCategorySaveRequest *catSaveRequest;
    QLandmarkSaveRequest *lmSaveRequest;
    QLandmarkCategoryFetchRequest *catFetchRequest;
    QLandmarkFetchRequest *lmFetchRequest;
    QLandmarkCategoryRemoveRequest *catRemoveRequest;
    QLandmarkRemoveRequest *lmRemoveRequest;
    QLandmarkImportRequest *lmImportRequest;
    QLandmarkExportRequest *lmExportRequest;

    QLandmarkManager *landmarkManager;
    QLandmarkCategoryId categoryId;
    QLandmarkCategory category;
    int previousLastIndex;
};

#endif
