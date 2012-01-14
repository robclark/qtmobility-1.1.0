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

#ifndef DATABASEOPERATIONS_P_H
#define DATABASEOPERATIONS_P_H

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

#include <qlandmarkmanager.h>
#include <QRunnable>
#include <QMap>

QTM_USE_NAMESPACE

QTM_BEGIN_NAMESPACE
class QLandmark;
class QLandmarkAbstractRequest;
class QLandmarkCategory;
class QLandmarkSortOrder;
class QLandmarkNameSort;
class QLandmarkFilter;
QTM_END_NAMESPACE

typedef QMap<int, QLandmarkManager::Error>  ERROR_MAP;
class QLandmarkManagerEngineSqlite;
class QLandmarkFileHandlerGpx;

class QueryRun : public QRunnable
{
public:
    QueryRun(QLandmarkAbstractRequest *req,
             const QString &uri,
             QLandmarkManagerEngineSqlite *eng,
             unsigned int runId);
    ~QueryRun();
    void run();

    QLandmarkAbstractRequest *request;
    QString connectionName;
    QLandmarkManager::Error error;
    QString errorString;
    QMap<int, QLandmarkManager::Error> errorMap;
    QString managerUri;
    volatile bool isCanceled;
    volatile bool isDeleted;
    QLandmarkManagerEngineSqlite *engine;
    unsigned int runId;
};

class DatabaseOperations {
    public:
    QString connectionName;
    QString managerUri;
    QueryRun *queryRun;

    DatabaseOperations();
    QLandmark retrieveLandmark(const QLandmarkId &landmarkId,
                               QLandmarkManager::Error *error, QString *errorString) const;

    QList<QLandmarkId> landmarkIds(const QLandmarkFilter& filter,
                                   const QList<QLandmarkSortOrder>& sortOrders,
                                   int limit, int offset,
                                   QLandmarkManager::Error *error, QString *errorString) const;

    QList<QLandmark> landmarks(const QLandmarkFilter& filter,
                               const QList<QLandmarkSortOrder>& sortOrders,
                               int limit, int offset,
                               QLandmarkManager::Error *error, QString *errorString) const ;

    QList<QLandmark> landmarks(const QList<QLandmarkId> &landmarkIds,
                               QMap<int, QLandmarkManager::Error> *errorMap,
                               QLandmarkManager::Error *error, QString *errorString) const;

    bool saveLandmarkHelper(QLandmark* landmark,
                      QLandmarkManager::Error *error, QString *errorString);

    bool saveLandmark(QLandmark* landmark,
                      QLandmarkManager::Error *error, QString *errorString);

    bool saveLandmarks(QList<QLandmark> * landmark,
                       QMap<int, QLandmarkManager::Error> *errorMap,
                       QLandmarkManager::Error *error, QString *errorString);

    bool removeLandmark(const QLandmarkId &landmarkId,
                        QLandmarkManager::Error *error, QString *errorString);

    bool removeLandmarks(const QList<QLandmarkId> &landmarkIds,
                         QMap<int, QLandmarkManager::Error> *errorMap,
                         QLandmarkManager::Error *error,
                         QString *errorString);

    QList<QLandmarkCategoryId> categoryIds(const QLandmarkNameSort &nameSort,
                                           int limit, int offet,
                                           QLandmarkManager::Error *error, QString *errorString) const;

    QLandmarkCategory category(const QLandmarkCategoryId &landmarkCategoryId,
                               QLandmarkManager::Error *error,
                               QString *errorString) const;

    QList<QLandmarkCategory> categories(const QList<QLandmarkCategoryId> &landmarkCategoryIds,
                                        const QLandmarkNameSort &nameSort,
                                        int limit, int offset,
                                        QLandmarkManager::Error *error, QString *errorString,
                                        bool needAll) const;

    QList<QLandmarkCategory> categories(const QList<QLandmarkCategoryId> &landmarkCategoryIds,
                                        QMap<int, QLandmarkManager::Error> *errorMap,
                                        QLandmarkManager::Error *error, QString *errorString) const;

    bool saveCategoryHelper(QLandmarkCategory *category,
                      QLandmarkManager::Error *error, QString *errorString);

    bool saveCategory(QLandmarkCategory *category,
                      QLandmarkManager::Error *error, QString *errorString);

    bool saveCategories(QList<QLandmarkCategory> * categories,
                        QMap<int, QLandmarkManager::Error> *errorMap,
                        QLandmarkManager::Error *error, QString *errorString);

    bool removeCategoryHelper(const QLandmarkCategoryId &categoryId,
                        QLandmarkManager::Error *error,
                        QString *errorString);

    bool removeCategory(const QLandmarkCategoryId &categoryId,
                        QLandmarkManager::Error *error,
                        QString *errorString);

    bool removeCategories(const QList<QLandmarkCategoryId> &categoryIds,
                          QMap<int, QLandmarkManager::Error> *errorMap,
                          QLandmarkManager::Error *error,
                          QString *errorString);

    bool importLandmarks(QIODevice *device,
                         const QString &format,
                         QLandmarkManager::TransferOption option,
                         const QLandmarkCategoryId &categoryId,
                         QLandmarkManager::Error *error,
                         QString *errorString,
                         QueryRun *queryRun =0,
                         QList<QLandmarkId> *landmarkIds = 0);

    bool exportLandmarks(QIODevice *device,
                         const QString &format,
                         const QList<QLandmarkId> &landmarkIds,
                         QLandmarkManager::TransferOption,
                         QLandmarkManager::Error *error,
                         QString *errorString) const;

    bool importLandmarksLmx(QIODevice *device,
                            QLandmarkManager::TransferOption option,
                            const QLandmarkCategoryId &categoryId,
                            QLandmarkManager::Error *error,
                            QString *errorString,
                            QueryRun *queryRun=0,
                            QList<QLandmarkId> *landmarkIds = 0);

    bool importLandmarksGpx(QIODevice *device,
                            QLandmarkManager::TransferOption option,
                            const QLandmarkCategoryId &categoryId,
                            QLandmarkManager::Error *error,
                            QString *errorString,
                            QueryRun *queryRun =0,
                            QList<QLandmarkId> *landmarkIds = 0);

    bool exportLandmarksLmx(QIODevice *device,
                            const QList<QLandmarkId> &landmarkIds,
                            QLandmarkManager::TransferOption option,
                            QLandmarkManager::Error *error,
                            QString *errorString) const ;

    bool exportLandmarksGpx(QIODevice *device,
                            const QList<QLandmarkId> &landmarkIds,
                            QLandmarkManager::Error *error,
                            QString *errorString) const;

    QLandmarkManager::SupportLevel filterSupportLevel(const QLandmarkFilter &filter) const;
    QLandmarkManager::SupportLevel sortOrderSupportLevel(const QLandmarkSortOrder &sortOrders) const;

    static const QStringList coreAttributes;
    static const QStringList coreGenericAttributes;
    static const QStringList supportedSearchableAttributes;
    static const QStringList coreCategoryAttributes;
    static const QStringList coreGenericCategoryAttributes;
};

#endif