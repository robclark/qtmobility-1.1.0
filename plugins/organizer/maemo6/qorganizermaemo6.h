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

#ifndef QORGANIZERMAEMO6_H
#define QORGANIZERMAEMO6_H


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

#include <QSharedData>
#include <QMap>
#include <QMultiMap>
#include <QList>
#include <QQueue>
#include <QPair>
#include <QSet>
#include <QDateTime>
#include <QString>
#include <QObject>

#include <extendedcalendar.h>

#include "qorganizeritem.h"
#include "qorganizermanager.h"
#include "qorganizermanagerengine.h"
#include "qorganizermanagerenginefactory.h"
#include "qorganizeritemdetaildefinition.h"
#include "qorganizerabstractrequest.h"
#include "qorganizeritemchangeset.h"

QTM_BEGIN_NAMESPACE
class QOrganizerEvent;
class QOrganizerTodo;
class QOrganizerNote;
class QOrganizerJournal;
class QOrganizerItemRecurrence;
class QOrganizerRecurrenceRule;
QTM_END_NAMESPACE

QTM_USE_NAMESPACE
using namespace KCal;

class QOrganizerItemMaemo6Factory : public QObject, public QOrganizerManagerEngineFactory
{
  Q_OBJECT
  Q_INTERFACES(QtMobility::QOrganizerManagerEngineFactory)
  public:
    QOrganizerManagerEngine* engine(const QMap<QString, QString>& parameters, QOrganizerManager::Error*);
    QString managerName() const;
    QOrganizerItemEngineLocalId* createItemEngineLocalId() const;
    QOrganizerCollectionEngineLocalId* createCollectionEngineLocalId() const;
};

class QOrganizerItemMaemo6EngineData : public QSharedData
{
public:
    QOrganizerItemMaemo6EngineData()
        : QSharedData(),
        m_calendarBackend(KDateTime::Spec::LocalZone())
    {
    }

    ~QOrganizerItemMaemo6EngineData()
    {
    }

    // map of organizeritem type to map of definition name to definitions:
    mutable QMap<QString, QMap<QString, QOrganizerItemDetailDefinition> > m_definitions;

    ExtendedCalendar m_calendarBackend;
};

class QOrganizerItemMaemo6Engine : public QOrganizerManagerEngine
{
    Q_OBJECT

public:
    ~QOrganizerItemMaemo6Engine();

    /* URI reporting */
    QString managerName() const;
    QMap<QString, QString> managerParameters() const;
    int managerVersion() const;

    QList<QOrganizerItem> itemInstances(const QOrganizerItem& generator, const QDateTime& periodStart, const QDateTime& periodEnd, int maxCount, QOrganizerManager::Error* error) const;
    QList<QOrganizerItemLocalId> itemIds(const QOrganizerItemFilter& filter, const QList<QOrganizerItemSortOrder>& sortOrders, QOrganizerManager::Error* error) const;
    QList<QOrganizerItem> items(const QOrganizerItemFilter& filter, const QList<QOrganizerItemSortOrder>& sortOrders, const QOrganizerItemFetchHint& fetchHint, QOrganizerManager::Error* error) const;
    QOrganizerItem item(const QOrganizerItemLocalId& itemId, const QOrganizerItemFetchHint& fetchHint, QOrganizerManager::Error* error) const;

    bool saveItems(QList<QOrganizerItem>* items, const QOrganizerCollectionLocalId& collectionId, QMap<int, QOrganizerManager::Error>* errorMap, QOrganizerManager::Error* error);
    bool saveItem(QOrganizerItem* item, QOrganizerManager::Error* error);
    bool removeItems(const QList<QOrganizerItemLocalId>& itemIds, QMap<int, QOrganizerManager::Error>* errorMap, QOrganizerManager::Error* error);

    /* Definitions - Accessors and Mutators */
    QMap<QString, QOrganizerItemDetailDefinition> detailDefinitions(const QString& itemType, QOrganizerManager::Error* error) const;
    QOrganizerItemDetailDefinition detailDefinition(const QString& definitionId, const QString& itemType, QOrganizerManager::Error* error) const;
    bool saveDetailDefinition(const QOrganizerItemDetailDefinition& def, const QString& itemType, QOrganizerManager::Error* error);
    bool removeDetailDefinition(const QString& definitionId, const QString& itemType, QOrganizerManager::Error* error);

    /* Collections - every item belongs to exactly one collection */
    QOrganizerCollectionLocalId defaultCollectionId(QOrganizerManager::Error* error) const;
    QList<QOrganizerCollectionLocalId> collectionIds(QOrganizerManager::Error* error) const;
    QList<QOrganizerCollection> collections(const QList<QOrganizerCollectionLocalId>& collectionIds, QMap<int, QOrganizerManager::Error>* errorMap, QOrganizerManager::Error* error) const;
    bool saveCollection(QOrganizerCollection* collection, QOrganizerManager::Error* error);
    bool removeCollection(const QOrganizerCollectionLocalId& collectionId, QOrganizerManager::Error* error);

    /* Capabilities reporting */
    bool hasFeature(QOrganizerManager::ManagerFeature feature, const QString& itemType) const;
    bool isFilterSupported(const QOrganizerItemFilter& filter) const;
    QList<int> supportedDataTypes() const;
    QStringList supportedItemTypes() const;

    /* Asynchronous Request Support */
    void requestDestroyed(QOrganizerAbstractRequest* req);
    bool startRequest(QOrganizerAbstractRequest* req);
    bool cancelRequest(QOrganizerAbstractRequest* req);
    bool waitForRequestFinished(QOrganizerAbstractRequest* req, int msecs);

private:
    QOrganizerItemMaemo6Engine();
    QMap<QString, QMap<QString, QOrganizerItemDetailDefinition> > schemaDefinitions() const;
    Incidence* incidence(const QOrganizerItemLocalId& itemId) const;
    Incidence* softSaveItem(QOrganizerItem* item, QOrganizerManager::Error* error);
    Event* createKEvent(const QOrganizerEvent& note);
    Todo* createKTodo(const QOrganizerTodo& note);
    Journal* createKJournal(const QOrganizerJournal& note);
    Journal* createKNote(const QOrganizerNote& note);
    void convertCommonDetailsToIncidenceFields(const QOrganizerItem& item, Incidence* incidence);
    void convertQRecurrenceToKRecurrence(const QOrganizerItemRecurrence& qRecurrence,
            Recurrence* kRecurrence);
    RecurrenceRule* createKRecurrenceRule(Recurrence* kRecurrence,
            const QOrganizerRecurrenceRule& rrule);

    QOrganizerItemMaemo6EngineData* d;

    friend class QOrganizerItemMaemo6Factory;

    class IncidenceToItemConverter {
    public:
        IncidenceToItemConverter(const QString managerUri)
            : m_managerUri(managerUri), m_visitor(this) {}
        bool convertIncidenceToItem(Incidence* incidence, QOrganizerItem* item);
        bool addEvent(Event* e);
        bool addTodo(Todo* t);
        bool addJournal(Journal* j);
    private:
        void convertCommonDetails(Incidence* incidence, QOrganizerItem* item);

        QString m_managerUri;
        QOrganizerItem m_converted;
        Incidence::AddVisitor<IncidenceToItemConverter> m_visitor;
    };
};

#endif

