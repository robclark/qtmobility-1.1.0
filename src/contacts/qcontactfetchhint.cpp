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

#include "qcontactfetchhint.h"
#include "qcontactfetchhint_p.h"

#include <QStringList>

QTM_BEGIN_NAMESPACE

/*!
  \class QContactFetchHint

  \inmodule QtContacts

  \brief The QContactFetchHint class provides hints to the manager about which contact
  information needs to be retrieved in an asynchronous fetch request or a synchronous
  function call.

  All of the hints may be ignored at the discretion of the manager, however if a manager
  is able to optimize retrieval of contacts due to hints, it may do so.  If a manager
  ignores a hint, it must retrieve the full set of data that the hint refers to.

  The fetch hint contains:
  \list
   \o a list of detail definition names which the client is interested
  in (empty if interested in all detail definitions)
   \o a list of relationship types which the client is interested in
  (empty if interested in all relationships)
   \o some optimization flags which allow the client to tell the backend if they are
  not interested in any relationships, any action preferences, or any binary blobs (images etc).
   \o a preferred size for any images, if the backend supports multiple sizes or scaling
  \endlist

  Important note: a client should not make changes to a contact which has been retrieved
  using a fetch hint other than the default fetch hint.  Doing so can result in information
  loss when saving the contact back to the manager (as the "new" restricted contact will
  replace the previously saved contact in the backend).
 */

/*!
  \enum QContactFetchHint::OptimizationHint

  This enum defines flags which may be set to inform the backend that the client does
  not require certain information.  The backend may safely ignore the hint, but then
  must return the full set of information relating to the optimization hint.

  \value AllRequired Tells the backend that all information is required
  \value NoRelationships Tells the backend that the client does not require retrieved contacts to include a cache of relationships
  \value NoActionPreferences Tells the backend that the client does not require retrieved contacts to include a cache of action preferences
  \value NoBinaryBlobs Tells the backend that the client does not require retrieved contacts to include binary blobs such as thumbnail images
 */

/*!
  Constructs a new contact fetch hint which requests that the backend fetch all information
 */
QContactFetchHint::QContactFetchHint()
    : d(new QContactFetchHintPrivate)
{
}

/*!
  Constructs a new contact fetch hint as a copy of \a other
 */
QContactFetchHint::QContactFetchHint(const QContactFetchHint &other)
    : d(other.d)
{
}

/*!
  Frees any memory in use by the fetch hint
 */
QContactFetchHint::~QContactFetchHint()
{
}

/*!
  Assigns this fetch hint to be equal to the \a other fetch hint
 */
QContactFetchHint& QContactFetchHint::operator=(const QContactFetchHint& other)
{
    d = other.d;
    return *this;
}

/*!
  Returns the list of definition names that identify detail definitions of which details
  the manager should (at a minimum) retrieve when fetching contacts.
  This hint may be ignored by the backend, in which case it will return the full set of details for
  each contact retrieved.

  \sa setDetailDefinitionsHint()
 */
QStringList QContactFetchHint::detailDefinitionsHint() const
{
    return d->m_definitionsHint;
}

/*!
  Sets the list of definition names that identify detail definitions of which details
  the manager should (at a minimum) retrieve when fetching contacts to \a definitionNames.
  This hint may be ignored by the backend, in which case it will return the full set of details for
  each contact retrieved.

  \sa detailDefinitionsHint()
 */
void QContactFetchHint::setDetailDefinitionsHint(const QStringList& definitionNames)
{
    d->m_definitionsHint = definitionNames;
}

/*!
  Returns the list of relationship types that the manager should (at a minimum) retrieve
  when fetching contacts.
  This hint may be ignored by the backend, in which case it will return the full set of
  relationships for each contact retrieved.

  \sa setRelationshipTypesHint(), QContact::relationships()
 */
QStringList QContactFetchHint::relationshipTypesHint() const
{
    return d->m_relationshipsHint;
}

/*!
  Sets the list of relationship types that the manager should (at a minimum) retrieve
  when fetching contacts to \a relationshipTypes.
  This hint may be ignored by the backend, in which case it will return the full set of
  relationships for each contact retrieved.

  \sa relationshipTypesHint(), QContact::relationships()
 */
void QContactFetchHint::setRelationshipTypesHint(const QStringList& relationshipTypes)
{
    d->m_relationshipsHint = relationshipTypes;
}

/*!
  Returns the preferred pixel dimensions for any images returned
  by the manager for a given request.  This hint may be ignored
  by the manager.

  This is useful when the backend supports multiple sizes of an
  image (or the image is natively scaleable) in order to get an
  image that will look good at the indicated dimensions.

  The caller should be prepared for images of any dimensions,
  in any case.
 */
QSize QContactFetchHint::preferredImageSize() const
{
    return d->m_preferredImageSize;
}

/*!
  Sets the preferred pixel dimensions for any images returned
  by the manager for the given request to \a size.  This hint
  may be ignored by the manager.

  This is useful when the backend supports multiple sizes of an
  image (or the image is natively scaleable) in order to get an
  image that will look good at the indicated dimensions.

  The caller should be prepared for images of any dimensions,
  in any case.
 */
void QContactFetchHint::setPreferredImageSize(const QSize& size)
{
    d->m_preferredImageSize = size;
}

/*!
  Returns the optimization hint flags specified by the client.
  These hints may be ignored by the backend, in which case it will return
  the full set of information accessible in a contact, including
  relationships, action preferences, and binary blobs.

  \sa setOptimizationHints()
 */
QContactFetchHint::OptimizationHints QContactFetchHint::optimizationHints() const
{
    return d->m_optimizationHints;
}

/*!
  Sets the optimization hint flags specified by the client to \a hints.
  These hints may be ignored by the backend, in which case it will return
  the full set of information accessible in a contact, including
  relationships, action preferences, and binary blobs.

  \sa optimizationHints()
 */
void QContactFetchHint::setOptimizationHints(OptimizationHints hints)
{
    d->m_optimizationHints = hints;
}

#ifndef QT_NO_DATASTREAM
QDataStream& operator<<(QDataStream& out, const QContactFetchHint& hint)
{
    quint8 formatVersion = 1; // Version of QDataStream format for QContactFetchHint
    return out << formatVersion
               << hint.detailDefinitionsHint()
               << hint.relationshipTypesHint()
               << static_cast<quint32>(hint.optimizationHints())
               << hint.preferredImageSize();
}

QDataStream& operator>>(QDataStream& in, QContactFetchHint& hint)
{
    hint = QContactFetchHint();
    quint8 formatVersion;
    in >> formatVersion;
    if (formatVersion == 1) {
        QStringList detailDefinitionHints;
        QStringList relationshipTypeHints;
        quint32 optimizations;
        QSize dimensions;
        in >> detailDefinitionHints >> relationshipTypeHints >> optimizations >> dimensions;
        hint.setDetailDefinitionsHint(detailDefinitionHints);
        hint.setRelationshipTypesHint(relationshipTypeHints);
        hint.setOptimizationHints(QContactFetchHint::OptimizationHints(optimizations));
        hint.setPreferredImageSize(dimensions);
    } else {
        in.setStatus(QDataStream::ReadCorruptData);
    }
    return in;
}

#endif

QTM_END_NAMESPACE
