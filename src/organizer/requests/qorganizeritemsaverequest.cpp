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

#include "qorganizeritemsaverequest.h"
#include "qorganizeritemrequests_p.h"

QTM_BEGIN_NAMESPACE

/*!
  \class QOrganizerItemSaveRequest
  \brief The QOrganizerItemSaveRequest class allows a client to asynchronously
    request that certain organizer items be saved to an organizer item store.
  \inmodule QtOrganizer

  For a QOrganizerItemSaveRequest, the resultsAvailable() signal will be emitted when
  either the individual item errors (which may be retrieved by calling errorMap()), or the resultant
  organizer items (which may be retrieved by calling items()), are updated, as well as if
  the overall operation error (which may be retrieved by calling error()) is updated.

  \ingroup organizer-requests
 */

/*! Constructs a new organizer item save request whose parent is the specified \a parent */
QOrganizerItemSaveRequest::QOrganizerItemSaveRequest(QObject* parent)
    : QOrganizerAbstractRequest(new QOrganizerItemSaveRequestPrivate, parent)
{
}

/*!
  Sets the organizer item to be saved to \a organizeritem.
  Equivalent to calling:
  \code
      setOrganizerItems(QList<QOrganizerItem>() << organizeritem);
  \endcode
 */
void QOrganizerItemSaveRequest::setItem(const QOrganizerItem& organizeritem)
{
    Q_D(QOrganizerItemSaveRequest);
    d->m_organizeritems.clear();
    d->m_organizeritems.append(organizeritem);
}

/*! Sets the list of organizer items to be saved to \a organizeritems */
void QOrganizerItemSaveRequest::setItems(const QList<QOrganizerItem>& organizeritems)
{
    Q_D(QOrganizerItemSaveRequest);
    d->m_organizeritems = organizeritems;
}

/*! Returns the list of organizer items which will be saved if called prior to calling \c start(),
    otherwise returns the list of organizer items as they were saved in the organizer item store */
QList<QOrganizerItem> QOrganizerItemSaveRequest::items() const
{
    Q_D(const QOrganizerItemSaveRequest);
    return d->m_organizeritems;
}

/*! Returns the map of input definition list indices to errors which occurred */
QMap<int, QOrganizerManager::Error> QOrganizerItemSaveRequest::errorMap() const
{
    Q_D(const QOrganizerItemSaveRequest);
    return d->m_errors;
}

#include "moc_qorganizeritemsaverequest.cpp"

QTM_END_NAMESPACE
