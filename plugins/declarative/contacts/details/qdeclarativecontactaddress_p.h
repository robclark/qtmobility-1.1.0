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


#ifndef QDECLARATIVECONTACTADDRESS_H
#define QDECLARATIVECONTACTADDRESS_H

#include "qdeclarativecontactdetail_p.h"
#include "qcontactaddress.h"

#include <QSet>

class QDeclarativeContactAddress : public QDeclarativeContactDetail
{
    Q_OBJECT

    Q_PROPERTY(QString street READ street WRITE setStreet NOTIFY fieldsChanged)
    Q_PROPERTY(QString locality READ locality WRITE setLocality NOTIFY fieldsChanged)
    Q_PROPERTY(QString region READ region WRITE setRegion  NOTIFY fieldsChanged)
    Q_PROPERTY(QString postcode READ postcode WRITE setPostcode  NOTIFY fieldsChanged)
    Q_PROPERTY(QString country READ country WRITE setCountry  NOTIFY fieldsChanged)
    Q_PROPERTY(QVariantList subTypes READ subTypes WRITE setSubTypes NOTIFY fieldsChanged)
    Q_PROPERTY(QString postOfficeBox READ postOfficeBox WRITE setPostOfficeBox  NOTIFY fieldsChanged)
    Q_ENUMS(FieldType)
    Q_ENUMS(AddressSubType)
public:
    enum FieldType {
        Street = 0,
        Locality,
        Region,
        PostCode,
        Country,
        SubTypes,
        PostOfficeBox
    };

    enum AddressSubType {
        Parcel = 0,
        Postal,
        Domestic,
        International
    };

    QDeclarativeContactAddress(QObject* parent = 0)
        :QDeclarativeContactDetail(parent)
    {
        setDetail(QContactAddress());
        connect(this, SIGNAL(fieldsChanged()), SIGNAL(valueChanged()));
    }

    ContactDetailType detailType() const
    {
        return QDeclarativeContactDetail::Address;
    }

    static QString fieldNameFromFieldType(int fieldType)
    {
        switch (fieldType) {
        case Street:
            return QContactAddress::FieldStreet;
        case Locality:
            return QContactAddress::FieldLocality;
        case Region:
            return QContactAddress::FieldRegion;
        case PostCode:
            return QContactAddress::FieldPostcode;
        case Country:
            return QContactAddress::FieldCountry;
        case SubTypes:
            return QContactAddress::FieldSubTypes;
        case PostOfficeBox:
            return QContactAddress::FieldPostOfficeBox;
        default:
            break;
        }
        //qWarning
        return QString();
    }
    void setStreet(const QString& v)
    {
        if (!readOnly() && v != street()) {
            detail().setValue(QContactAddress::FieldStreet, v);
            emit fieldsChanged();
        }
    }
    QString street() const {return detail().value(QContactAddress::FieldStreet);}
    void setLocality(const QString& v)
    {
        if (!readOnly() && v != locality()) {
            detail().setValue(QContactAddress::FieldLocality, v);
            emit fieldsChanged();
        }
    }
    QString locality() const {return detail().value(QContactAddress::FieldLocality);}
    void setRegion(const QString& v)
    {
        if (!readOnly() && v != region()) {
            detail().setValue(QContactAddress::FieldRegion, v);
            emit fieldsChanged();
        }
    }
    QString region() const {return detail().value(QContactAddress::FieldRegion);}
    void setPostcode(const QString& v)
    {
        if (!readOnly() && v != postcode()) {
            detail().setValue(QContactAddress::FieldPostcode, v);
            emit fieldsChanged();
        }
    }
    QString postcode() const {return detail().value(QContactAddress::FieldPostcode);}
    void setCountry(const QString& v)
    {
        if (!readOnly() && v != country()) {
            detail().setValue(QContactAddress::FieldCountry, v);
            emit fieldsChanged();
        }
    }
    QString country() const {return detail().value(QContactAddress::FieldCountry);}
    void setPostOfficeBox(const QString& v)
    {
        if (!readOnly() && v != postOfficeBox()) {
            detail().setValue(QContactAddress::FieldPostOfficeBox, v);
            emit fieldsChanged();
        }
    }
    QString postOfficeBox() const {return detail().value(QContactAddress::FieldPostOfficeBox);}

    void setSubTypes(const QVariantList& subTypes)
    {
        QStringList savedList;
        foreach (const QVariant subType, subTypes) {
            switch (static_cast<AddressSubType>(subType.value<int>()))
            {
            case Parcel:
                savedList << QContactAddress::SubTypeParcel;
                break;
            case Postal:
                savedList << QContactAddress::SubTypePostal;
                break;
            case Domestic:
                savedList << QContactAddress::SubTypeDomestic;
                break;
            case International:
                savedList << QContactAddress::SubTypeInternational;
                break;
            default:
                //qWarning unknown type
                break;

            }
        }

        QStringList oldList = detail().value<QStringList>(QContactAddress::FieldSubTypes);

        if (!readOnly() && savedList.toSet() != oldList.toSet()) {
            detail().setValue(QContactAddress::FieldSubTypes, savedList);
            emit fieldsChanged();
        }
    }

    QVariantList subTypes() const
    {
        QVariantList returnList;
        QStringList savedList = detail().value<QStringList>(QContactAddress::FieldSubTypes);
        foreach (const QString& subType, savedList) {
            if (subType == QContactAddress::SubTypePostal)
                returnList << static_cast<int>(Postal);
            else if (subType == QContactAddress::SubTypeParcel)
                returnList << static_cast<int>(Parcel);
            else if (subType == QContactAddress::SubTypeDomestic)
                returnList << static_cast<int>(Domestic);
            else if (subType == QContactAddress::SubTypeInternational)
                returnList << static_cast<int>(International);
        }
        return returnList;
    }
signals:
    void fieldsChanged();
};

QML_DECLARE_TYPE(QDeclarativeContactAddress)

#endif

