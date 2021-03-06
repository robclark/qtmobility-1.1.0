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

#include "qcontactdetaildefinition.h"
#include "qcontactdetaildefinition_p.h"

QTM_BEGIN_NAMESPACE

/*!
  \class QContactDetailDefinition

  \inmodule QtContacts
  
  The QContactDetailDefinition class provides the specification for
  a detail that can be included in any particular QContact.
  The definition does not include any data, but defines
  the semantics of the representation and use of data
  details that are stored in a QContact.
 */

/*!
  \fn QContactDetailDefinition::operator!=(const QContactDetailDefinition& other) const
  Returns true if this detail definition has different allowable field types or uniqueness to the \a other definition
 */

/*! Construct a new, invalid QContactDetailDefinition */
QContactDetailDefinition::QContactDetailDefinition()
    : d(new QContactDetailDefinitionData)
{
}

/*! Constructs a new QContactDetailDefinition from \a other */
QContactDetailDefinition::QContactDetailDefinition(const QContactDetailDefinition& other)
    : d(other.d)
{
}

/*! Assigns this QContactDetailDefinition to \a other */
QContactDetailDefinition& QContactDetailDefinition::operator=(const QContactDetailDefinition& other)
{
    d = other.d;
    return *this;
}

/*! Frees the memory used by this QContactDetailDefinition */
QContactDetailDefinition::~QContactDetailDefinition()
{
}

/*! Returns true if the definition has the same type, uniqueness and allowable value datatypes as \a other */
bool QContactDetailDefinition::operator==(const QContactDetailDefinition& other) const
{
    if (d->m_name != other.d->m_name)
        return false;
    if (d->m_unique != other.d->m_unique)
        return false;
    if (d->m_fields != other.d->m_fields)
        return false;
    return true;
}

/*! Returns true if the id and fields of this definition are empty */
bool QContactDetailDefinition::isEmpty() const
{
    if (!d->m_name.isEmpty())
        return false;
    if (!d->m_fields.isEmpty())
        return false;
    return true;
}

#ifndef QT_NO_DATASTREAM
/*!
 * Writes \a definition to the stream \a out.
 */
QDataStream& operator<<(QDataStream& out, const QContactDetailDefinition& definition)
{
    quint8 formatVersion = 1; // Version of QDataStream format for QContactDetailDefinition
    return out << formatVersion
               << definition.name()
               << definition.isUnique()
               << definition.fields();
}

/*!
 * Reads a detail definition from stream \a in into \a definition.
 */
QDataStream& operator>>(QDataStream& in, QContactDetailDefinition& definition)
{
    definition = QContactDetailDefinition();
    quint8 formatVersion;
    in >> formatVersion;
    if (formatVersion == 1) {
        QString name;
        bool unique;
        QMap<QString, QContactDetailFieldDefinition> fields;
        in >> name >> unique >> fields;
        definition.setName(name);
        definition.setUnique(unique);
        definition.setFields(fields);
    } else {
        in.setStatus(QDataStream::ReadCorruptData);
    }
    return in;
}
#endif

/*! Sets the unique identifier of this detail type to \a definitionName. */
void QContactDetailDefinition::setName(const QString& definitionName)
{
    d->m_name = definitionName;
}

/*!
  Sets whether a contact can have more than one detail of this type.
  If \a unique is true, only one detail of this type can be added.
  Otherwise, any number can be added.
 */
void QContactDetailDefinition::setUnique(bool unique)
{
    d->m_unique = unique;
}

/*! Returns the identifier of this detail definition */
QString QContactDetailDefinition::name() const
{
    return d->m_name;
}

/*! Returns the per-contact uniqueness of this definition */
bool QContactDetailDefinition::isUnique() const
{
    return d->m_unique;
}

/*! Sets the fields which constitute the data of details of this this definition to \a fields */
void QContactDetailDefinition::setFields(const QMap<QString, QContactDetailFieldDefinition>& fields)
{
    d->m_fields = fields;
}

/*! Returns the map of keys to fields which are present in details of this definition */
QMap<QString, QContactDetailFieldDefinition> QContactDetailDefinition::fields() const
{
    return d->m_fields;
}

/*! Inserts the field \a field into the map of fields which constitute the data of details of this definition for the given field key \a key.
    If another field for that key already exists in the definition, it will be overwritten. */
void QContactDetailDefinition::insertField(const QString& key, const QContactDetailFieldDefinition& field)
{
    d->m_fields.insert(key, field);
}

/*! Removes the field associated with the given field key \a key from the map of fields which constitute the data of details of this definition. */
void QContactDetailDefinition::removeField(const QString& key)
{
    d->m_fields.remove(key);
}


QTM_END_NAMESPACE
