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


#include "qdeclarativedeviceinfo_p.h"
#include "qsystemdeviceinfo.h"
#include <QMetaType>
#include <QDebug>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QSystemDeviceInfo, deviceInfo)

/*!
    \qmlclass DeviceInfo QDeclarativeDeviceInfo
    \brief The DeviceInfo element allows you to receive notifications from the device.

    This element is part of the \bold{QtMobility.systeminfo 1.0} module.
    It is a convience class to make QML usage easier.

    Note: To use notification signals, you need to use the start* slots.


    \qml
        Component.onCompleted: {
            deviceinfo.startPowerStateChanged();
            deviceinfo.startBatteryLevelChanged();
        }
    \endqml

\sa QSystemDeviceInfo
*/

/*!
    \qmlsignal DeviceInfo::batteryLevelChanged()

    This handler is called when battery level has changed.
    Note: To receive this notification, you must first call \a startBatteryLevelChanged.
*/
/*!
    \qmlsignal DeviceInfo::batteryStatusChanged()

    This handler is called when battery status has changed.
    Note: To receive this notification, you must first call \a startBatteryStatusChanged.
*/
/*!
    \qmlsignal DeviceInfo::powerStateChanged()

    This handler is called when the power state has changed.
    Note: To receive this notification, you must first call \a startPowerStateChanged.
*/
/*!
    \qmlsignal DeviceInfo::currentProfileChanged()

    This handler is called when current device profile has changed.
    Note: To receive this notification, you must first call \a startCurrentProfileChanged.
*/
/*!
    \qmlsignal DeviceInfo::bluetoothStateChanged()

    This handler is called when bluetooth power states has changed.
    Note: To receive this notification, you must first call \a startBluetoothStateChanged.
*/

/*!
    \internal
    \class QDeclarativeDeviceInfo
    \brief The QDeclarativeDeviceInfo class provides a DeviceInfo item that you can add to a QDeclarativeView.
*/

QDeclarativeDeviceInfo::QDeclarativeDeviceInfo(QObject *parent) :
    QSystemDeviceInfo(parent)
{
}

/*!
    \qmlmethod DeviceInfo::startBatteryLevelChanged()
   This function is needed to start batteryLevelChanged notification

   \sa connectNotify()
*/

void QDeclarativeDeviceInfo::startBatteryLevelChanged()
{
    connect(deviceInfo(),SIGNAL(batteryLevelChanged(int)),
            this,SLOT(declarativeBatteryLevelChanged(int)),Qt::UniqueConnection);
}

/*!
    \qmlmethod DeviceInfo::startBatteryStatusChanged()
   This function is needed to start batteryStatusChanged notification

   \sa connectNotify()
*/
void QDeclarativeDeviceInfo::startBatteryStatusChanged()
{
    connect(deviceInfo(),SIGNAL(batteryStatusChanged(QSystemDeviceInfo::BatteryStatus)),
            this,SLOT(declarativeBatteryStatusChanged(QSystemDeviceInfo::BatteryStatus)),Qt::UniqueConnection);
}

/*!
    \qmlmethod DeviceInfo::startPowerStateChanged()
   This function is needed to start powerStateChanged notification

   \sa connectNotify()
*/
void QDeclarativeDeviceInfo::startPowerStateChanged()
{
    qDebug() << Q_FUNC_INFO;

    connect(deviceInfo(),SIGNAL(powerStateChanged(QSystemDeviceInfo::PowerState)),
            this,SLOT(declarativePowerStateChanged(QSystemDeviceInfo::PowerState)),Qt::UniqueConnection);
}

/*!
    \qmlmethod DeviceInfo::startCurrentProfileChanged()
   This function is needed to start currentProfileChanged notification

   \sa connectNotify()
*/
void QDeclarativeDeviceInfo::startCurrentProfileChanged()
{
    connect(deviceInfo(),SIGNAL(currentProfileChanged(QSystemDeviceInfo::Profile)),
            this,SLOT(declarativeCurrentProfileChanged(QSystemDeviceInfo::Profile)),Qt::UniqueConnection);
}

/*!
    \qmlmethod DeviceInfo::startBluetoothStateChanged()
   This function is needed to start bluetoothStateChanged notification

   \sa connectNotify()
*/
void QDeclarativeDeviceInfo::startBluetoothStateChanged()
{
    connect(deviceInfo(),SIGNAL(bluetoothStateChanged(bool)),
            this,SLOT(declarativeBluetoothStateChanged(bool)),Qt::UniqueConnection);
}

/*!
   \internal

   This function is called when the client connects from the networkSignalStrengthChanged()
   notification.

   \sa connectNotify()
*/
void QDeclarativeDeviceInfo::declarativeBatteryLevelChanged(int level)
{
    Q_EMIT batteryLevelChanged(level);
}

/*!
   \internal

   This function is called when the client connects from the batteryStatusChanged()
   notification.

   \sa connectNotify()
*/
void QDeclarativeDeviceInfo::declarativeBatteryStatusChanged(QSystemDeviceInfo::BatteryStatus batteryStatus)
{
    Q_EMIT batteryStatusChanged(batteryStatus);
}

/*!
   \internal

   This function is called when the client connects from the powerStateChanged()
   notification.

   \sa connectNotify()
*/
void QDeclarativeDeviceInfo::declarativePowerStateChanged(QSystemDeviceInfo::PowerState powerState)
{
    Q_EMIT powerStateChanged(powerState);
}

/*!
   \internal

   This function is called when the client connects from the currentProfileChanged()
   notification.

   \sa connectNotify()
*/
void QDeclarativeDeviceInfo::declarativeCurrentProfileChanged(QSystemDeviceInfo::Profile currentProfile)
{
    Q_EMIT currentProfileChanged(currentProfile);
}

/*!
   \internal

   This function is called when the client connects from the bluetoothStateChanged()
   notification.

   \sa connectNotify()
*/
void QDeclarativeDeviceInfo::declarativeBluetoothStateChanged(bool on)
{
    Q_EMIT bluetoothStateChanged(on);
}


