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

#ifdef SYMBIAN_BACKEND_USE_SQLITE

#include "cnttransformfavorite.h"

QList<CContactItemField *> CntTransformFavorite::transformDetailL(const QContactDetail &detail)
{
    if(detail.definitionName() != QContactFavorite::DefinitionName) {
       User::Leave(KErrArgument);
    }

    QList<CContactItemField *> fieldList;

	//cast to favorite
	const QContactFavorite &favorite(static_cast<const QContactFavorite&>(detail));
	if (!favorite.isFavorite()) {
	    //do not add favorite field
	    return fieldList;
	}
	
    CContactItemField* itemField = CContactItemField::NewLC(KStorageTypeContactItemId, KUidContactFieldFavourite);
    itemField->AgentStorage()->SetAgentId(favorite.index());
    itemField->SetMapping(KUidContactFieldVCardMapFavourite);
    fieldList.append(itemField);
    CleanupStack::Pop(itemField);

	return fieldList;
}

QContactDetail *CntTransformFavorite::transformItemField(const CContactItemField& field, const QContact &contact)
{
	Q_UNUSED(contact);

	CContactAgentField* storage = field.AgentStorage();
	QContactFavorite *favoriteDetail = new QContactFavorite();
	favoriteDetail->setFavorite(true);
	favoriteDetail->setIndex(storage->Value());
	return favoriteDetail;
}

bool CntTransformFavorite::supportsDetail(QString detailName) const
{
    bool ret = false;
    if (detailName == QContactFavorite::DefinitionName) {
        ret = true;
    }
    return ret;
}

QList<TUid> CntTransformFavorite::supportedFields() const
{
    return QList<TUid>() << KUidContactFieldFavourite;
}

QList<TUid> CntTransformFavorite::supportedSortingFieldTypes(QString detailFieldName) const
{
    QList<TUid> uids;
    if (detailFieldName == QContactFavorite::FieldFavorite) {
        uids << KUidContactFieldFavourite;
    }
    return uids;
}

/*!
 * Checks whether the subtype is supported
 *
 * \a subType The subtype to be checked
 * \return True if this subtype is supported
 */
bool CntTransformFavorite::supportsSubType(const QString& subType) const
{
    Q_UNUSED(subType);
    return false;
}

/*!
 * Returns the filed id corresponding to a field
 *
 * \a fieldName The name of the supported field
 * \return fieldId for the fieldName, 0  if not supported
 */
quint32 CntTransformFavorite::getIdForField(const QString& fieldName) const
{
   if (QContactFavorite::FieldFavorite == fieldName) {
       return KUidContactFieldFavourite.iUid;
   }
   else {
       return 0;
   }
}

/*!
 * Modifies the detail definitions. The default detail definitions are
 * queried from QContactManagerEngine::schemaDefinitions and then modified
 * with this function in the transform leaf classes.
 *
 * \a definitions The detail definitions to modify.
 * \a contactType The contact type the definitions apply for.
 */
void CntTransformFavorite::detailDefinitions(QMap<QString, QContactDetailDefinition> &definitions, const QString& contactType) const
{
    Q_UNUSED(contactType);

    if (definitions.contains(QContactFavorite::DefinitionName)) {
        QContactDetailDefinition d = definitions.value(QContactFavorite::DefinitionName);
        QMap<QString, QContactDetailFieldDefinition> fields = d.fields();

        // Context not supported in symbian back-end, remove
        fields.remove(QContactFavorite::FieldContext);

        d.setFields(fields);

        // Replace original definitions
        definitions.insert(d.name(), d);
    }
}

#endif
