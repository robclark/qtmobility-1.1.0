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

//TESTED_COMPONENT=src/systeminfo

#include <QtTest/QtTest>
#include "qsysteminfo.h"
#include <QDesktopWidget>

QTM_USE_NAMESPACE
class tst_QSystemDisplayInfo : public QObject
{
    Q_OBJECT

private slots:
    void tst_displayBrightness();
    void tst_colorDepth();

};

void tst_QSystemDisplayInfo::tst_displayBrightness()
{
    QSystemDisplayInfo di;
    QVERIFY(di.displayBrightness(0) > -2);
    QVERIFY(di.displayBrightness(999) == -1);
}

void tst_QSystemDisplayInfo::tst_colorDepth()
{
    QSystemDisplayInfo di;
    int depth = di.colorDepth(0);
    QDesktopWidget wid;
    if(wid.screenCount() > 1) {
        QVERIFY(depth == 0
                || depth == 8
                || depth == 16
                || depth == 24
                || depth == 32
                || depth == 64);
        }
    QVERIFY(di.colorDepth(999) == -1);
}


QTEST_MAIN(tst_QSystemDisplayInfo)
#include "tst_qsystemdisplayinfo.moc"