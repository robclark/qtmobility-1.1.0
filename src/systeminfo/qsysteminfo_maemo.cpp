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
#include "qsysteminfocommon_p.h"
#include <qsysteminfo_maemo_p.h>
#include <QStringList>
#include <QSize>
#include <QFile>
#include <QTextStream>
#include <QLocale>
#include <QLibraryInfo>
//#include <QtGui>
#include <QDesktopWidget>
#include <QDebug>
#include <QTimer>
#include <QDir>
#include <QTimer>
#include <QMapIterator>

#if !defined(QT_NO_DBUS)
#include "gconfitem_p.h" // Temporarily here.
#endif

#ifdef Q_WS_X11
#include <QX11Info>
#include <X11/Xlib.h>

#endif

#include <QDBusInterface>

#if !defined(QT_NO_DBUS)
QDBusArgument &operator<<(QDBusArgument &argument, const ProfileDataValue &value)
{
  argument.beginStructure();
  argument << value.key << value.val << value.type;
  argument.endStructure();
  return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ProfileDataValue &value)
{
  argument.beginStructure();
  argument >> value.key >> value.val >> value.type;
  argument.endStructure();
  return argument;
}
#endif

QTM_BEGIN_NAMESPACE

QSystemInfoPrivate::QSystemInfoPrivate(QSystemInfoLinuxCommonPrivate *parent)
 : QSystemInfoLinuxCommonPrivate(parent)
{
}

QSystemInfoPrivate::~QSystemInfoPrivate()
{
}

QStringList QSystemInfoPrivate::availableLanguages() const
{
    QStringList languages;

#if !defined(Q_WS_MAEMO_6)
    GConfItem languagesItem("/apps/osso/inputmethod/available_languages");
#else
    GConfItem languagesItem("/meegotouch/inputmethods/languages");
#endif
    const QStringList locales = languagesItem.value().toStringList();

    foreach(const QString locale, locales) {
        languages << locale.mid(0,2);
    }
    languages << currentLanguage();
    languages.removeDuplicates();

    return languages;
}

QString QSystemInfoPrivate::currentLanguage() const
{
#if defined(Q_WS_MAEMO_6)
    GConfItem langItem("/meegotouch/i18n/language");
    return langItem.value().toString();
#else
    return QSystemInfoLinuxCommonPrivate::currentLanguage();
#endif
}


QString QSystemInfoPrivate::currentCountryCode() const
{
#if defined(Q_WS_MAEMO_6)
    GConfItem langItem("/meegotouch/i18n/language");
    return langItem.value().toString().section("_",1,1);
#else
    return QSystemInfoLinuxCommonPrivate::currentCountryCode();
#endif
}

QString QSystemInfoPrivate::version(QSystemInfo::Version type,
                                    const QString &parameter)
{
    QString errorStr = "Not Available";

    switch(type) {
        case QSystemInfo::Firmware :
        {
            QDBusInterface connectionInterface("com.nokia.SystemInfo",
                                               "/com/nokia/SystemInfo",
                                               "com.nokia.SystemInfo",
                                               QDBusConnection::systemBus());
            QDBusReply< QByteArray > reply =
                connectionInterface.call("GetConfigValue",
                                         "/device/sw-release-ver");
            if(reply.isValid())
                return reply.value();
            break;
        }
        default:
            return QSystemInfoLinuxCommonPrivate::version(type, parameter);
            break;
    };
    return errorStr;
}

bool QSystemInfoPrivate::hasFeatureSupported(QSystemInfo::Feature feature)
{
    bool featureSupported = false;
    switch (feature) {
    case QSystemInfo::SimFeature :
        {
            QSystemDeviceInfoPrivate d;
            featureSupported = (d.simStatus() != QSystemDeviceInfo::SimNotAvailable);
        }
        break;
    case QSystemInfo::LocationFeature :
        {
#if defined(Q_WS_MAEMO_6)
            GConfItem satellitePositioning("/system/osso/location/settings/satellitePositioning");
            GConfItem networkPositioning("/system/osso/location/settings/networkPositioning");

            bool satellitePositioningAvailable = satellitePositioning.value(false).toBool();
            bool networkPositioningAvailable   = networkPositioning.value(false).toBool();

            featureSupported = (satellitePositioningAvailable || networkPositioningAvailable);
#else /* Maemo 5 */
            GConfItem locationValues("/system/nokia/location");
            const QStringList locationKeys = locationValues.listEntries();
            if(locationKeys.count()) {
                featureSupported = true;
            }
#endif /* Maemo 5 */
        }
        break;
    case QSystemInfo::HapticsFeature:
        {
           // if(halIsAvailable) {
                QHalInterface iface;
                const QStringList touchSupport =
                        iface.findDeviceByCapability("input.touchpad");
                if(touchSupport.count()) {
                    featureSupported = true;
                } else {
                    featureSupported = false;
                }
            }
      //  }
        break;
    default:
        featureSupported = QSystemInfoLinuxCommonPrivate::hasFeatureSupported(feature);
        break;
    };
    return featureSupported;
}

QSystemNetworkInfoPrivate::QSystemNetworkInfoPrivate(QSystemNetworkInfoLinuxCommonPrivate *parent)
        : QSystemNetworkInfoLinuxCommonPrivate(parent)
{
    csStatusMaemo6["Unknown"]    = -1;  // Current registration status is unknown.
    csStatusMaemo6["Home"]       = 0;   // Registered with the home network.
    csStatusMaemo6["Roaming"]    = 1;   // Registered with a roaming network.
    csStatusMaemo6["Offline"]    = 3;   // Not registered.
    csStatusMaemo6["Searching"]  = 4;   // Offline, but currently searching for network.
    csStatusMaemo6["NoSim"]      = 6;   // Offline because no SIM is present.
    csStatusMaemo6["PowerOff"]   = 8;   // Offline because the CS is powered off.
    csStatusMaemo6["PowerSave"]  = 9;   // Offline and in power save mode.
    csStatusMaemo6["NoCoverage"] = 10;  // Offline and in power save mode because of poor coverage.
    csStatusMaemo6["Rejected"]   = 11;  // Offline because SIM was rejected by the network.

    QTimer::singleShot(0,this,SLOT(setupNetworkInfo()));
    QCoreApplication::processEvents();//this needs to be called otherwise this timer never fires
}

QSystemNetworkInfoPrivate::~QSystemNetworkInfoPrivate()
{
    if(wlanSignalStrengthTimer->isActive())
        wlanSignalStrengthTimer->stop();
}

QSystemNetworkInfo::NetworkStatus QSystemNetworkInfoPrivate::networkStatus(QSystemNetworkInfo::NetworkMode mode)
{
    switch(mode) {
    case QSystemNetworkInfo::GsmMode:
    case QSystemNetworkInfo::CdmaMode:
    case QSystemNetworkInfo::WcdmaMode:
        {
            // radioAccessTechnology: 1 = GSM, 2 = WCDMA
            if((radioAccessTechnology == 1 && mode != QSystemNetworkInfo::GsmMode) ||
               (radioAccessTechnology == 2 && mode != QSystemNetworkInfo::WcdmaMode)) {
                return QSystemNetworkInfo::NoNetworkAvailable;
            }

            switch(currentCellNetworkStatus) {
                case 0: return QSystemNetworkInfo::HomeNetwork; // CS is registered to home network
                case 1: return QSystemNetworkInfo::Roaming; // CS is registered to some other network than home network
                case 2: return QSystemNetworkInfo::Roaming; // CS is registered to non-home system in a non-home area
                case 3: return QSystemNetworkInfo::NoNetworkAvailable; // CS is not in service
                case 4: return QSystemNetworkInfo::Searching; // CS is not in service, but is currently searching for service
                case 5: return QSystemNetworkInfo::NoNetworkAvailable; // CS is not in service and it is not currently searching for service
                case 6: return QSystemNetworkInfo::NoNetworkAvailable; // CS is not in service due to missing SIM or missing subscription
                case 8: return QSystemNetworkInfo::NoNetworkAvailable; // CS is in power off state
                case 9: return QSystemNetworkInfo::NoNetworkAvailable; // CS is in No Service Power Save State (currently not listening to any cell)
                case 10: return QSystemNetworkInfo::NoNetworkAvailable; // CS is in No Service Power Save State (CS is entered to this state
                                                                        // because there is no network coverage)
                case 11: return QSystemNetworkInfo::Denied; // CS is not in service due to missing subscription
                default:
                    break;
            };
        }
        break;
    case QSystemNetworkInfo::EthernetMode:
        if(currentEthernetState == "up") {
            return QSystemNetworkInfo::Connected;
        } else {
            return QSystemNetworkInfo::NoNetworkAvailable;
        }
        break;
    case QSystemNetworkInfo::WlanMode:
    case QSystemNetworkInfo::BluetoothMode:
        {
            return QSystemNetworkInfoLinuxCommonPrivate::networkStatus(mode);
        }
        break;
    default:
        break;
    };
    return QSystemNetworkInfo::UndefinedStatus;
}

qint32 QSystemNetworkInfoPrivate::networkSignalStrength(QSystemNetworkInfo::NetworkMode mode)
{
    switch(mode) {
    case QSystemNetworkInfo::GsmMode:
    case QSystemNetworkInfo::CdmaMode:
    case QSystemNetworkInfo::WcdmaMode:
    {
            // radioAccessTechnology: 1 = GSM, 2 = WCDMA
            if((radioAccessTechnology == 1 && mode != QSystemNetworkInfo::GsmMode) ||
               (radioAccessTechnology == 2 && mode != QSystemNetworkInfo::WcdmaMode)) {
                return -1;
            }

            return cellSignalStrength;
    }
    case QSystemNetworkInfo::EthernetMode: {
        QString result;
        QString baseSysDir = "/sys/class/net/";
        QString interface = QSystemNetworkInfoLinuxCommonPrivate::interfaceForMode(mode).humanReadableName();
        if (interface == "usb0") {
            QString dir = QSystemNetworkInfoLinuxCommonPrivate::interfaceForMode(mode).name();
            QString devFile = baseSysDir + dir;
            QFileInfo fi(devFile + "/carrier");
            if(fi.exists()) {
                QFile rx(fi.absoluteFilePath());
                if(rx.exists() && rx.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream stream(&rx);
                    stream >> result;
                    rx.close();
                    return result.toInt() * 100;
                    break;
                }
            }
        }
        return QSystemNetworkInfoLinuxCommonPrivate::networkSignalStrength(mode);
        break;
    }
    case QSystemNetworkInfo::WlanMode:
    case QSystemNetworkInfo::BluetoothMode:
        return QSystemNetworkInfoLinuxCommonPrivate::networkSignalStrength(mode);
        break;
    default:
        break;
    };

    return -1;
}

int QSystemNetworkInfoPrivate::cellId()
{
    return currentCellId;
}

int QSystemNetworkInfoPrivate::locationAreaCode()
{
    return currentLac;
}

QString QSystemNetworkInfoPrivate::currentMobileCountryCode()
{
    return currentMCC;
}

QString QSystemNetworkInfoPrivate::currentMobileNetworkCode()
{
    return currentMNC;
}

QString QSystemNetworkInfoPrivate::homeMobileCountryCode()
{
    QSystemDeviceInfoPrivate d;
    QString imsi = d.imsi();
    if (imsi.length() >= 3) {
        return imsi.left(3);
    }
    return "";
}

QString QSystemNetworkInfoPrivate::homeMobileNetworkCode()
{
#if !defined(QT_NO_DBUS)
    #if defined(Q_WS_MAEMO_6)
    QDBusInterface connectionInterface("com.nokia.csd.SIM",
                                       "/com/nokia/csd/sim",
                                       "com.nokia.csd.SIM.Identity",
                                       QDBusConnection::systemBus());
    QDBusMessage reply = connectionInterface.call(QLatin1String("GetHPLMN"));
    if (reply.errorName().isEmpty()) {
        QList<QVariant> args = reply.arguments();
        // The first attribute should be MCC and the 2nd one MNC
        if (args.size() == 2) {
            return args.at(1).toString();
        }
    }
    #else
    /* Maemo 5 */
    QDBusInterface connectionInterface("com.nokia.phone.SIM",
                                       "/com/nokia/phone/SIM",
                                       "Phone.Sim",
                                       QDBusConnection::systemBus());
    if (!connectionInterface.isValid()) {
        qWarning() << "interface not valid";
        return QString();
    }
    QDBusReply<QByteArray> reply = connectionInterface.call(QLatin1String("read_hplmn"));

    // The MNC and MCC are split into Hex numbers in the received byte array.
    // The MNC can be 2 or 3 digits long. If it is 2 digits long, it ends with 0xF.
    // The order of the Hex numbers in the reply is:
    // mcc2 mcc1 mnc3 mcc3 mnc2 mnc1

    QString homeMobileNetworkCode;
    if (reply.isValid()) {
        QString temp = reply.value().toHex();
        QString mnc1 = temp.right(1);
        temp.chop(1);
        QString mnc2 = temp.right(1);
        temp.chop(2);
        QString mnc3 = temp.right(1);
        if (mnc3 != "f") {
            homeMobileNetworkCode.prepend(mnc3);
        }
        homeMobileNetworkCode.prepend(mnc2);
        homeMobileNetworkCode.prepend(mnc1);
        return homeMobileNetworkCode;
    }
    #endif
#endif
    return "";
}

QString QSystemNetworkInfoPrivate::networkName(QSystemNetworkInfo::NetworkMode mode)
{
    QString netname = "";

    switch(mode) {

    case QSystemNetworkInfo::CdmaMode:
    case QSystemNetworkInfo::GsmMode:
    case QSystemNetworkInfo::WcdmaMode: {
        // radioAccessTechnology: 1 = GSM, 2 = WCDMA
        if((radioAccessTechnology == 1 && mode != QSystemNetworkInfo::GsmMode) ||
           (radioAccessTechnology == 2 && mode != QSystemNetworkInfo::WcdmaMode)) {
            break;
        }

        return currentOperatorName;
        break;
    }
    case QSystemNetworkInfo::WimaxMode:
        break;
    default:
        return QSystemNetworkInfoLinuxCommonPrivate::networkName(mode);
        break;
    };
    return netname;
}

QString QSystemNetworkInfoPrivate::macAddress(QSystemNetworkInfo::NetworkMode mode)
{
    switch(mode) {

    case QSystemNetworkInfo::CdmaMode:
    case QSystemNetworkInfo::GsmMode:
    case QSystemNetworkInfo::WcdmaMode:
    case QSystemNetworkInfo::WimaxMode:
        break;
    case QSystemNetworkInfo::EthernetMode: {
        QString address;
        QString baseSysDir = "/sys/class/net/";
        QString interface = QSystemNetworkInfoLinuxCommonPrivate::interfaceForMode(mode).humanReadableName();
        if (interface == "usb0") {
            QString dir = QSystemNetworkInfoLinuxCommonPrivate::interfaceForMode(mode).name();
            QString devFile = baseSysDir + dir;
            QFileInfo fi(devFile + "/address");
            if(fi.exists()) {
                QFile rx(fi.absoluteFilePath());
                if(rx.exists() && rx.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream stream(&rx);
                    stream >> address;
                    rx.close();
                    return address;
                    break;
                }
            }
        }
        return QSystemNetworkInfoLinuxCommonPrivate::macAddress(mode);
        break;
    }
    default:
        return QSystemNetworkInfoLinuxCommonPrivate::macAddress(mode);
        break;
    };
    return QString();
}

QNetworkInterface QSystemNetworkInfoPrivate::interfaceForMode(QSystemNetworkInfo::NetworkMode mode)
{
#if !defined(QT_NO_DBUS)
    switch(mode) {
    case QSystemNetworkInfo::CdmaMode:
    case QSystemNetworkInfo::GsmMode:
    case QSystemNetworkInfo::WcdmaMode:
    case QSystemNetworkInfo::WimaxMode:
        break;
    default:
        return QSystemNetworkInfoLinuxCommonPrivate::interfaceForMode(mode);
        break;
    };
#endif
    return QNetworkInterface();
}

void QSystemNetworkInfoPrivate::setupNetworkInfo()
{
    currentCellNetworkStatus = -1;
    currentBluetoothNetworkStatus = networkStatus(QSystemNetworkInfo::BluetoothMode);
    currentWlanNetworkStatus = networkStatus(QSystemNetworkInfo::WlanMode);
    currentEthernetState = "down";
    currentEthernetSignalStrength = networkSignalStrength(QSystemNetworkInfo::EthernetMode);
    currentWlanSignalStrength = networkSignalStrength(QSystemNetworkInfo::WlanMode);
    currentLac = -1;
    currentCellId = -1;
    currentMCC = "";
    currentMNC = "";
    cellSignalStrength = 0;
    currentOperatorName = "";
    radioAccessTechnology = 0;
    iWlanStrengthCheckEnabled = 0;
    wlanSignalStrengthTimer = new QTimer(this);

    connect(wlanSignalStrengthTimer, SIGNAL(timeout()), this, SLOT(wlanSignalStrengthCheck()));

    QString devFile = "/sys/class/net/usb0/operstate";
    QFileInfo fi(devFile);
    if (fi.exists()) {
        QFile rx(fi.absoluteFilePath());
        if (rx.exists() && rx.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&rx);
            stream >> currentEthernetState;
            rx.close();
        }
    }

    currentNetworkMode = currentMode();

#if !defined(QT_NO_DBUS)
    QDBusConnection systemDbusConnection = QDBusConnection::systemBus();

    #if defined(Q_WS_MAEMO_6)
        const QString service = "com.nokia.csd.CSNet";
        const QString servicePath = "/com/nokia/csd/csnet";

        /* CSD: network cell */
        QDBusInterface ifc(service, servicePath, "com.nokia.csd.CSNet.NetworkCell", systemDbusConnection);

        QVariant cellLac = ifc.property("CellLac");
        currentLac = cellLac.isValid() ? cellLac.value<int>() : -1;

        QVariant cellId = ifc.property("CellId");
        currentCellId =  cellId.isValid() ? cellId.value<int>() : -1;

        QVariant cellType = ifc.property("CellType");
        QString currentCellType = cellType.isValid() ? cellType.value<QString>() : "";

        if (currentCellType == "GSM")
            radioAccessTechnology = 1;
        else if (currentCellType == "WCDMA")
            radioAccessTechnology = 2;

        /* CSD: network operator */
        QDBusInterface ifc2(service, servicePath, "com.nokia.csd.CSNet.NetworkOperator", systemDbusConnection);

        QVariant mcc = ifc2.property("OperatorMCC");
        currentMCC = mcc.isValid() ? mcc.value<QString>() : "";

        QVariant mnc = ifc2.property("OperatorMNC");
        currentMNC = mnc.isValid() ? mnc.value<QString>() : "";

        QVariant operatorName = ifc2.property("OperatorName");
        currentOperatorName = operatorName.isValid() ? operatorName.value<QString>() : "";

        /* CSD: signal strength */
        QDBusInterface ifc3(service, servicePath, "com.nokia.csd.CSNet.SignalStrength", systemDbusConnection);

        QVariant signalStrength = ifc3.property("SignalPercent");
        cellSignalStrength = signalStrength.isValid() ? signalStrength.value<int>() : -1;

        /* CSD: network registration */
        QDBusInterface ifc4(service, servicePath, "com.nokia.csd.CSNet.NetworkRegistration", systemDbusConnection);

        QVariant registrationStatus = ifc4.property("RegistrationStatus");
        QString status = registrationStatus.isValid() ? registrationStatus.value<QString>() : "";

        currentCellNetworkStatus = csStatusMaemo6.value(status, -1);

        /* Signal handlers */
        if (!systemDbusConnection.connect(service, servicePath, "com.nokia.csd.CSNet.SignalStrength", "SignalStrengthChanged",
                                         this, SLOT(slotSignalStrengthChanged(int, int)))) {
            qDebug() << "unable to connect SignalStrengthChanged";
        }
        if (!systemDbusConnection.connect(service, servicePath, "com.nokia.csd.CSNet.NetworkOperator", "OperatorChanged",
                                         this, SLOT(slotOperatorChanged(const QString&,const QString&)))) {
            qDebug() << "unable to connect (OperatorChanged";
        }
        if (!systemDbusConnection.connect(service, servicePath, "com.nokia.csd.CSNet.NetworkOperator", "OperatorNameChanged",
                                         this, SLOT(slotOperatorNameChanged(const QString&)))) {
            qDebug() << "unable to connect OperatorNameChanged";
        }
        if (!systemDbusConnection.connect(service, servicePath, "com.nokia.csd.CSNet.NetworkRegistration", "RegistrationChanged",
                                         this, SLOT(slotRegistrationChanged(const QString&)))) {
            qDebug() << "unable to connect RegistrationChanged";
        }
        if (!systemDbusConnection.connect(service, servicePath, "com.nokia.csd.CSNet.NetworkCell", "CellChanged",
                                         this, SLOT(slotCellChanged(const QString&,int,int)))) {
            qDebug() << "unable to connect CellChanged";
        }
    #else
    /* Maemo 5 */
    QDBusInterface connectionInterface("com.nokia.phone.net",
                                       "/com/nokia/phone/net",
                                       "Phone.Net",
                                       systemDbusConnection);
    if (!connectionInterface.isValid()) {
        qWarning() << "setupNetworkInfo(): interface not valid";
        return;
    }
    QDBusMessage reply = connectionInterface.call(QLatin1String("get_registration_status"));
    if (reply.type() == QDBusMessage::ReplyMessage) {
        QList<QVariant> argList = reply.arguments();
        currentCellNetworkStatus = argList.at(STATUS_INDEX).toInt();
        currentLac = argList.at(LAC_INDEX).value<ushort>();
        currentCellId = argList.at(CELLID_INDEX).value<uint>();
        currentMCC.setNum(argList.at(MCC_INDEX).value<uint>());
        currentMNC.setNum(argList.at(MNC_INDEX).value<uint>());
    } else {
        qWarning() << reply.errorMessage();
    }
    if (!systemDbusConnection.connect("com.nokia.phone.net",
                       "/com/nokia/phone/net",
                       "Phone.Net",
                       "registration_status_change",
                       this, SLOT(registrationStatusChanged(uchar,ushort,uint,uint,uint,uchar,uchar)))) {
        qWarning() << "unable to connect to registration_status_change";
    }
    reply = connectionInterface.call(QLatin1String("get_signal_strength"));
    if (reply.type() == QDBusMessage::ReplyMessage) {
        QList<QVariant> argList = reply.arguments();
        cellSignalStrength = argList.at(0).toInt();
    } else {
        qWarning() << reply.errorMessage();
    }
    if (!systemDbusConnection.connect("com.nokia.phone.net",
                       "/com/nokia/phone/net",
                       "Phone.Net",
                       "signal_strength_change",
                       this, SLOT(cellNetworkSignalStrengthChanged(uchar,uchar)))) {
        qWarning() << "unable to connect to signal_strength_change";
    }
    uchar type = 0;
    QList<QVariant> argumentList;
    argumentList << qVariantFromValue(type) << qVariantFromValue(currentMNC.toUInt()) << qVariantFromValue(currentMCC.toUInt());
    reply = connectionInterface.callWithArgumentList(QDBus::Block, QLatin1String("get_operator_name"), argumentList);

    if (reply.type() == QDBusMessage::ReplyMessage) {
        QList<QVariant> argList = reply.arguments();
        currentOperatorName = argList.at(0).toString();
    } else {
        qWarning() << reply.errorMessage();
    }
    if (!systemDbusConnection.connect("com.nokia.phone.net",
                       "/com/nokia/phone/net",
                       "Phone.Net",
                       "operator_name_change",
                       this, SLOT(operatorNameChanged(uchar,QString,QString,uint,uint)))) {
        qWarning() << "unable to connect to operator_name_change";
    }

    reply = connectionInterface.call(QLatin1String("get_radio_access_technology"));
    if (reply.type() == QDBusMessage::ReplyMessage) {
        QList<QVariant> argList = reply.arguments();
        radioAccessTechnology = argList.at(0).toInt();
    } else {
        qWarning() << reply.errorMessage();
    }
    if (!systemDbusConnection.connect("com.nokia.phone.net",
                       "/com/nokia/phone/net",
                       "Phone.Net",
                       "radio_access_technology_change",
                       this, SLOT(networkModeChanged(int)))) {
        qWarning() << "unable to connect to radio_access_technology_change";
    }
    if(!systemDbusConnection.connect("com.nokia.bme",
                              "/com/nokia/bme/signal",
                              "com.nokia.bme.signal",
                              QLatin1String("charger_connected"),
                              this, SLOT(usbCableAction())) ) {
        qWarning() << "unable to connect to usbCableAction (connect)";
    }
    if(!systemDbusConnection.connect("com.nokia.bme",
                              "/com/nokia/bme/signal",
                              "com.nokia.bme.signal",
                              QLatin1String("charger_disconnected"),
                              this, SLOT(usbCableAction())) ) {
        qWarning() << "unable to connect to usbCableAction (disconnect)";
    }
    #endif /* Maemo 5 */

    if(!systemDbusConnection.connect("com.nokia.icd",
                              "/com/nokia/icd",
                              "com.nokia.icd",
                              QLatin1String("status_changed"),
                              this, SLOT(icdStatusChanged(QString,QString,QString,QString))) ) {
        qWarning() << "unable to connect to icdStatusChanged";
    }
    if(!systemDbusConnection.connect("org.freedesktop.Hal",
                              "/org/freedesktop/Hal/Manager",
                              "org.freedesktop.Hal.Manager",
                              QLatin1String("DeviceAdded"),
                              this, SLOT(bluetoothNetworkStatusCheck(QString))) ) {
        qWarning() << "unable to connect to bluetoothNetworkStatusCheck (1)";
    }
    if(!systemDbusConnection.connect("org.freedesktop.Hal",
                              "/org/freedesktop/Hal/Manager",
                              "org.freedesktop.Hal.Manager",
                              QLatin1String("DeviceRemoved"),
                              this, SLOT(bluetoothNetworkStatusCheck(QString))) ) {
        qWarning() << "unable to connect to bluetoothNetworkStatusCheck (2)";
    }
#endif
}

#if defined(Q_WS_MAEMO_6)
// Slots only available in Maemo6

void QSystemNetworkInfoPrivate::slotSignalStrengthChanged(int percent, int /*dbm*/)
{
    QSystemNetworkInfo::NetworkMode mode = QSystemNetworkInfo::UnknownMode;
    cellSignalStrength = percent;

    if (radioAccessTechnology == 1)
        mode = QSystemNetworkInfo::GsmMode;
    if (radioAccessTechnology == 2)
        mode = QSystemNetworkInfo::WcdmaMode;

    if (mode != QSystemNetworkInfo::UnknownMode)
        emit networkSignalStrengthChanged(mode, cellSignalStrength);
}

void QSystemNetworkInfoPrivate::slotOperatorChanged(const QString &mnc, const QString &mcc)
{
    if (currentMCC != mcc) {
        currentMCC = mcc;
        emit currentMobileCountryCodeChanged(currentMCC);
    }
    if (currentMNC != mnc) {
        currentMNC = mnc;
        emit currentMobileNetworkCodeChanged(currentMNC);
    }
}

void QSystemNetworkInfoPrivate::slotOperatorNameChanged(const QString &name)
{
    currentOperatorName = name;
    if (radioAccessTechnology == 1)
        emit networkNameChanged(QSystemNetworkInfo::GsmMode, currentOperatorName);
    if (radioAccessTechnology == 2)
        emit networkNameChanged(QSystemNetworkInfo::WcdmaMode, currentOperatorName);
}

void QSystemNetworkInfoPrivate::slotRegistrationChanged(const QString &status)
{
    int newCellNetworkStatus = csStatusMaemo6.value(status, -1);

    if (currentCellNetworkStatus != newCellNetworkStatus) {
        currentCellNetworkStatus = newCellNetworkStatus;
        if (radioAccessTechnology == 1)
            emit networkStatusChanged(QSystemNetworkInfo::GsmMode,
                                      networkStatus(QSystemNetworkInfo::GsmMode));
        if (radioAccessTechnology == 2)
            emit networkStatusChanged(QSystemNetworkInfo::WcdmaMode,
                                      networkStatus(QSystemNetworkInfo::WcdmaMode));
    }
}

void QSystemNetworkInfoPrivate::slotCellChanged(const QString &type, int id, int lac)
{
    int newRadioAccessTechnology = 0;
    if (type == "GSM") {
        newRadioAccessTechnology = 1;
    } else if (type == "WCDMA") {
        newRadioAccessTechnology = 2;
    }

    if (newRadioAccessTechnology != radioAccessTechnology) {
        radioAccessTechnology = newRadioAccessTechnology;
        networkModeChangeCheck();
    }
    if (currentCellId != id) {
        currentCellId = id;
    }
    if (currentLac != lac) {
        currentLac = lac;
    }
}

#endif /* Maemo 6 */

#if defined(Q_WS_MAEMO_5)
// Slots only available in Maemo5

void QSystemNetworkInfoPrivate::cellNetworkSignalStrengthChanged(uchar var1, uchar)
{
    QSystemNetworkInfo::NetworkMode mode = QSystemNetworkInfo::UnknownMode;
    cellSignalStrength = var1;

    if (radioAccessTechnology == 1)
        mode = QSystemNetworkInfo::GsmMode;
    if (radioAccessTechnology == 2)
        mode = QSystemNetworkInfo::WcdmaMode;

    if (mode != QSystemNetworkInfo::UnknownMode)
        emit networkSignalStrengthChanged(mode, cellSignalStrength);
}

void QSystemNetworkInfoPrivate::networkModeChanged(int newRadioAccessTechnology)
{
    radioAccessTechnology = newRadioAccessTechnology;
    networkModeChangeCheck();
}

void QSystemNetworkInfoPrivate::operatorNameChanged(uchar, QString name, QString, uint, uint)
{
    currentOperatorName = name;
    if (radioAccessTechnology == 1)
        emit networkNameChanged(QSystemNetworkInfo::GsmMode, currentOperatorName);
    if (radioAccessTechnology == 2)
        emit networkNameChanged(QSystemNetworkInfo::WcdmaMode, currentOperatorName);
}

void QSystemNetworkInfoPrivate::registrationStatusChanged(uchar var1, ushort var2, uint var3, uint var4, uint var5, uchar, uchar)
{
    int newCellNetworkStatus = var1;
    int newLac = var2;
    int newCellId = var3;
    QString newMobileCountryCode;
    QString newMobileNetworkCode;
    newMobileCountryCode.setNum(var4);
    newMobileNetworkCode.setNum(var5);

    if (currentCellNetworkStatus != newCellNetworkStatus) {
        currentCellNetworkStatus = newCellNetworkStatus;
        if (radioAccessTechnology == 1)
            emit networkStatusChanged(QSystemNetworkInfo::GsmMode,
                                      networkStatus(QSystemNetworkInfo::GsmMode));
        if (radioAccessTechnology == 2)
            emit networkStatusChanged(QSystemNetworkInfo::WcdmaMode,
                                      networkStatus(QSystemNetworkInfo::WcdmaMode));
    }
    if (currentLac != newLac) {
        currentLac = newLac;
    }
    if (currentCellId != newCellId) {
        currentCellId = newCellId;
    }
    if (currentMCC != newMobileCountryCode) {
        currentMCC = newMobileCountryCode;
        emit currentMobileCountryCodeChanged(currentMCC);
    }
    if (currentMNC != newMobileNetworkCode) {
        currentMNC = newMobileNetworkCode;
        emit currentMobileNetworkCodeChanged(currentMNC);
    }
}

#endif /* Maemo 5 */

void QSystemNetworkInfoPrivate::icdStatusChanged(QString, QString var2, QString, QString)
{
    if (var2 == "WLAN_INFRA") {
        QSystemNetworkInfo::NetworkStatus newWlanNetworkStatus =
                networkStatus(QSystemNetworkInfo::WlanMode);

        if (currentWlanNetworkStatus != newWlanNetworkStatus) {
            currentWlanNetworkStatus = newWlanNetworkStatus;
            emit networkStatusChanged(QSystemNetworkInfo::WlanMode, currentWlanNetworkStatus);
            networkModeChangeCheck();
        }
    }
}

void QSystemNetworkInfoPrivate::usbCableAction()
{
    if (currentEthernetSignalStrength != networkSignalStrength(QSystemNetworkInfo::EthernetMode)) {
        currentEthernetSignalStrength = networkSignalStrength(QSystemNetworkInfo::EthernetMode);
        emit networkSignalStrengthChanged(QSystemNetworkInfo::EthernetMode,
                                  currentEthernetSignalStrength);
    }
    QString newEthernetState;
    QString devFile = "/sys/class/net/usb0/operstate";
    QFileInfo fi(devFile);
    if (fi.exists()) {
        QFile rx(fi.absoluteFilePath());
        if (rx.exists() && rx.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&rx);
            stream >> newEthernetState;
            rx.close();
        }
    }

    if (currentEthernetState != newEthernetState) {
        currentEthernetState = newEthernetState;
        emit networkStatusChanged(QSystemNetworkInfo::EthernetMode,
                                  networkStatus(QSystemNetworkInfo::EthernetMode));
    }

    networkModeChangeCheck();
}

QSystemNetworkInfo::NetworkMode QSystemNetworkInfoPrivate::currentMode()
{
    if(networkStatus(QSystemNetworkInfo::EthernetMode) == QSystemNetworkInfo::Connected) {
        return QSystemNetworkInfo::EthernetMode;
    }
    if(networkStatus(QSystemNetworkInfo::WlanMode) == QSystemNetworkInfo::Connected) {
        return QSystemNetworkInfo::WlanMode;
    }

    if (radioAccessTechnology == 1)
        return QSystemNetworkInfo::GsmMode;
    if (radioAccessTechnology == 2)
        return QSystemNetworkInfo::WcdmaMode;

    return QSystemNetworkInfo::UnknownMode;
}

void QSystemNetworkInfoPrivate::networkModeChangeCheck()
{
    QSystemNetworkInfo::NetworkMode newNetworkMode = currentMode();
    if (currentNetworkMode != newNetworkMode)
    {
        currentNetworkMode = newNetworkMode;
        emit networkModeChanged(currentNetworkMode);
    }
}

void QSystemNetworkInfoPrivate::wlanSignalStrengthCheck()
{
    int strength = 0;
    strength =  networkSignalStrength(QSystemNetworkInfo::WlanMode);
    if (currentWlanSignalStrength != strength) {
        currentWlanSignalStrength = strength;
        emit networkSignalStrengthChanged(QSystemNetworkInfo::WlanMode, currentWlanSignalStrength);
    }
}

void QSystemNetworkInfoPrivate::bluetoothNetworkStatusCheck(QString device)
{
    if (device == "/org/freedesktop/Hal/devices/net_1b") {
        QTimer::singleShot(500, this, SLOT(usbCableAction()));
    } else {
        if (currentBluetoothNetworkStatus != networkStatus(QSystemNetworkInfo::BluetoothMode)) {
            currentBluetoothNetworkStatus = networkStatus(QSystemNetworkInfo::BluetoothMode);
            emit networkStatusChanged(QSystemNetworkInfo::BluetoothMode, currentBluetoothNetworkStatus);
        }
    }
}


void QSystemNetworkInfoPrivate::setWlanSignalStrengthCheckEnabled(bool enabled)
{
    if (enabled) {
        iWlanStrengthCheckEnabled++;
        if (!wlanSignalStrengthTimer->isActive())
            wlanSignalStrengthTimer->start(5000); //5 seconds interval
    } else {
        iWlanStrengthCheckEnabled--;
        if (iWlanStrengthCheckEnabled <= 0) {
            if(wlanSignalStrengthTimer->isActive())
                wlanSignalStrengthTimer->stop();
        }
    }
}

QSystemDisplayInfoPrivate::QSystemDisplayInfoPrivate(QSystemDisplayInfoLinuxCommonPrivate *parent)
        : QSystemDisplayInfoLinuxCommonPrivate(parent)
{
}

QSystemDisplayInfoPrivate::~QSystemDisplayInfoPrivate()
{
}

int QSystemDisplayInfoPrivate::displayBrightness(int screen)
{
    QDesktopWidget wid;
    if(wid.screenCount() - 1 < screen) {
        return -1;
    }
    GConfItem currentBrightness("/system/osso/dsm/display/display_brightness");
    GConfItem maxBrightness("/system/osso/dsm/display/max_display_brightness_levels");
    if(maxBrightness.value().toInt()) {
        float retVal = 100 * (currentBrightness.value().toFloat() /
                              maxBrightness.value().toFloat());
        return retVal;
    }

    return -1;
}

QSystemStorageInfoPrivate::QSystemStorageInfoPrivate(QSystemStorageInfoLinuxCommonPrivate *parent)
        : QSystemStorageInfoLinuxCommonPrivate(parent)
{
}

QSystemStorageInfoPrivate::~QSystemStorageInfoPrivate()
{
}

QSystemDeviceInfoPrivate::QSystemDeviceInfoPrivate(QSystemDeviceInfoLinuxCommonPrivate *parent)
        : QSystemDeviceInfoLinuxCommonPrivate(parent)
{
    flightMode = false;
 #if !defined(QT_NO_DBUS)
    previousPowerState = QSystemDeviceInfo::UnknownPower;
    setupProfile();
#endif
}

QSystemDeviceInfoPrivate::~QSystemDeviceInfoPrivate()
{
}

#if !defined(QT_NO_DBUS)
void QSystemDeviceInfoPrivate::halChanged(int,QVariantList map)
{
    for(int i=0; i < map.count(); i++) {
       qWarning() << __FUNCTION__ << map.at(i).toString();
       if(map.at(i).toString() == "battery.charge_level.percentage") {
            int level = batteryLevel();
            emit batteryLevelChanged(level);
            QSystemDeviceInfo::BatteryStatus stat = QSystemDeviceInfo::NoBatteryLevel;

            if(level < 4) {
                stat = QSystemDeviceInfo::BatteryCritical;
            } else if(level < 11) {
                stat = QSystemDeviceInfo::BatteryVeryLow;
            } else if(level < 41) {
                stat = QSystemDeviceInfo::BatteryLow;
            } else if(level > 40) {
                stat = QSystemDeviceInfo::BatteryNormal;
            }
            if(currentBatStatus != stat) {
                currentBatStatus = stat;
                Q_EMIT batteryStatusChanged(stat);
            }
        }
        if((map.at(i).toString() == "maemo.charger.connection_status")
        || (map.at(i).toString() == "maemo.rechargeable.charging_status")) {
            QSystemDeviceInfo::PowerState state = currentPowerState();
            if (previousPowerState != state)
                emit powerStateChanged(state);
            previousPowerState = state;
       }
    } //end map
}
#endif

QSystemDeviceInfo::Profile QSystemDeviceInfoPrivate::currentProfile()
{
#if !defined(QT_NO_DBUS)
    if (flightMode)
        return QSystemDeviceInfo::OfflineProfile;

    if (silentProfile)
        return vibratingAlertEnabled ? QSystemDeviceInfo::VibProfile : QSystemDeviceInfo::SilentProfile;

    if (beepProfile)
        return QSystemDeviceInfo::BeepProfile;

    if (ringingAlertVolume > 75)
        return QSystemDeviceInfo::LoudProfile;

    return QSystemDeviceInfo::NormalProfile;
#endif

    return QSystemDeviceInfo::UnknownProfile;
}

QString QSystemDeviceInfoPrivate::imei()
{
#if !defined(QT_NO_DBUS)
    #if defined(Q_WS_MAEMO_6)
        QString dBusService = "com.nokia.csd.Info";
    #else
        /* Maemo 5 */
        QString dBusService = "com.nokia.phone.SIM";
    #endif
    QDBusInterface connectionInterface(dBusService,
                                       "/com/nokia/csd/info",
                                       "com.nokia.csd.Info",
                                        QDBusConnection::systemBus());
    QDBusReply< QString > reply = connectionInterface.call("GetIMEINumber");
    return reply.value();
#endif
    return "";
}

QString QSystemDeviceInfoPrivate::imsi()
{
#if defined(Q_WS_MAEMO_6)
    /* Maemo 6 */
    #if !defined(QT_NO_DBUS)
        QDBusInterface connectionInterface("com.nokia.csd.SIM",
                                           "/com/nokia/csd/sim",
                                           "com.nokia.csd.SIM.Identity",
                                           QDBusConnection::systemBus());
        QDBusReply< QString > reply = connectionInterface.call("GetIMSI");
        return reply.value();
    #endif
    return "";
#else
    /* Maemo 5 */
    return GConfItem("/system/nokia/location/sim_imsi").value().toString();
#endif
}

QSystemDeviceInfo::SimStatus QSystemDeviceInfoPrivate::simStatus()
{
    QSystemDeviceInfo::SimStatus simStatus = QSystemDeviceInfo::SimNotAvailable;
    QString imsi = QSystemDeviceInfoPrivate::imsi();
    if (imsi.length() > 0) {
        simStatus = QSystemDeviceInfo::SingleSimAvailable;
    }
    return simStatus;
}

bool QSystemDeviceInfoPrivate::isDeviceLocked()
{
#if !defined(QT_NO_DBUS)
    QDBusConnection systemDbusConnection = QDBusConnection::systemBus();

    QDBusInterface mceConnectionInterface("com.nokia.mce",
                                      "/com/nokia/mce/request",
                                      "com.nokia.mce.request",
                                      systemDbusConnection);
    if (mceConnectionInterface.isValid()) {
        QDBusReply<QString> tkLockModeReply = mceConnectionInterface.call("get_tklock_mode");
        return tkLockModeReply.value() == "locked";
    }

    qWarning() << "mce interface not valid";
#endif
    return false;
}

QSystemDeviceInfo::PowerState QSystemDeviceInfoPrivate::currentPowerState()
{
#if !defined(QT_NO_DBUS)
        QHalInterface iface;
        const QStringList list = iface.findDeviceByCapability("battery");
        if(!list.isEmpty()) {
            foreach(const QString dev, list) {
                QHalDeviceInterface ifaceDevice(dev);
                if (iface.isValid()) {
                    if (ifaceDevice.getPropertyString("maemo.charger.connection_status") == "connected") {
                        if (ifaceDevice.getPropertyString("maemo.rechargeable.charging_status") == "full")
                            return QSystemDeviceInfo::WallPower;
                        return QSystemDeviceInfo::WallPowerChargingBattery;
                    }
                    return QSystemDeviceInfo::BatteryPower;
                }
            }
        }
#endif
    return QSystemDeviceInfo::UnknownPower;
}

#if !defined(QT_NO_DBUS)
 void QSystemDeviceInfoPrivate::setupBluetooth()
 {
     QDBusConnection dbusConnection = QDBusConnection::systemBus();
     QDBusInterface *connectionInterface;
     connectionInterface = new QDBusInterface("org.bluez",
                                              "/",
                                              "org.bluez.Manager",
                                              dbusConnection);
     if (connectionInterface->isValid()) {

         QDBusReply<  QDBusObjectPath > reply = connectionInterface->call("DefaultAdapter");
         if (reply.isValid()) {
             QDBusInterface *adapterInterface;
             adapterInterface = new QDBusInterface("org.bluez",
                                                   reply.value().path(),
                                                   "org.bluez.Adapter",
                                                   dbusConnection);
             if (adapterInterface->isValid()) {
                 if (!dbusConnection.connect("org.bluez",
                                           reply.value().path(),
                                            "org.bluez.Adapter",
                                            "PropertyChanged",
                                            this,SLOT(bluezPropertyChanged(QString, QDBusVariant)))) {
                     qWarning() << "bluez could not connect signal";
                 }
             }
         }
     }
 }
#endif

#if !defined(QT_NO_DBUS)
 void QSystemDeviceInfoPrivate::bluezPropertyChanged(const QString &str, QDBusVariant v)
 {
     //qWarning() << str << v.variant().toBool();
     emit bluetoothStateChanged(v.variant().toBool());
 }
#endif

#if !defined(QT_NO_DBUS)

void QSystemDeviceInfoPrivate::setupProfile()
{
    qDebug() << Q_FUNC_INFO;
    QDBusConnection systemDbusConnection = QDBusConnection::systemBus();

    QDBusInterface mceConnectionInterface("com.nokia.mce",
                                      "/com/nokia/mce/request",
                                      "com.nokia.mce.request",
                                      systemDbusConnection);
    if (!mceConnectionInterface.isValid()) {
        qWarning() << "mce interface not valid";
        return;
    } else {
        QDBusReply<QString> deviceModeReply = mceConnectionInterface.call("get_device_mode");
        flightMode = QString::compare(deviceModeReply.value(), "flight", Qt::CaseInsensitive) == 0;
    }

    if (!systemDbusConnection.connect("com.nokia.mce",
                           "/com/nokia/mce/signal",
                           "com.nokia.mce.signal",
                           "sig_device_mode_ind",
                           this, SLOT(deviceModeChanged(QString)))) {
        qWarning() << "unable to connect to sig_device_mode_ind";
    }


    QDBusInterface connectionInterface("com.nokia.profiled",
                                      "/com/nokia/profiled",
                                      "com.nokia.profiled",
                                      QDBusConnection::sessionBus());
    if(!connectionInterface.isValid()) {
       qWarning() << "profiled interface not valid";
       return;
    }

    QDBusReply<QString> profileNameReply = connectionInterface.call("get_profile");
    if (profileNameReply.isValid())
        profileName = profileNameReply.value();
qDebug() << Q_FUNC_INFO << profileName;

    QDBusReply<QString> ringingAlertTypeReply = connectionInterface.call("get_value", profileName, "ringing.alert.type");
    qDebug() << ringingAlertTypeReply.value();

    if (ringingAlertTypeReply.isValid()) {
        silentProfile = QString::compare(ringingAlertTypeReply.value(), "silent", Qt::CaseInsensitive) == 0;
        beepProfile = QString::compare(ringingAlertTypeReply.value(), "beep", Qt::CaseInsensitive) == 0;
    }

    QDBusReply<QString> vibratingAlertEnabledReply = connectionInterface.call("get_value", profileName, "vibrating.alert.enabled");
    if (vibratingAlertEnabledReply.isValid())
        vibratingAlertEnabled = QString::compare(vibratingAlertEnabledReply.value(), "On", Qt::CaseInsensitive) == 0;

    QDBusReply<QString> ringingAlertVolumeReply = connectionInterface.call("get_value", profileName, "ringing.alert.volume");
    if (ringingAlertVolumeReply.isValid())
        ringingAlertVolume = ringingAlertVolumeReply.value().toInt();

    qDBusRegisterMetaType<ProfileDataValue>();
    qDBusRegisterMetaType<QList<ProfileDataValue> >();

    QDBusConnection sessionDbusConnection = QDBusConnection::sessionBus();
    if (!sessionDbusConnection.connect("com.nokia.profiled",
                           "/com/nokia/profiled",
                           "com.nokia.profiled",
                           "profile_changed",
                           this, SLOT(profileChanged(bool, bool, QString, QList<ProfileDataValue>)))) {
        qWarning() << "unable to connect to profile_changed";
    }

}

void QSystemDeviceInfoPrivate::deviceModeChanged(QString newMode)
{
    bool previousFlightMode = flightMode;
    flightMode = newMode == "flight";
    if (previousFlightMode != flightMode)
        emit currentProfileChanged(currentProfile());
}

void QSystemDeviceInfoPrivate::profileChanged(bool changed, bool active, QString profile, QList<ProfileDataValue> values)
{
    qDebug() << __FUNCTION__;
    if (active) {
        profileName = profile;
        foreach (const ProfileDataValue value, values) {
            qDebug() << value.key << value.val;
            if (value.key == "ringing.alert.type") {
                silentProfile = QString::compare(value.val, "silent", Qt::CaseInsensitive) == 0;
                beepProfile = QString::compare(value.val, "beep", Qt::CaseInsensitive) == 0;
            }
            else if (value.key == "vibrating.alert.enabled")
                vibratingAlertEnabled = QString::compare(value.val, "On", Qt::CaseInsensitive) == 0;
            else if (value.key == "ringing.alert.volume")
                ringingAlertVolume = value.val.toInt();
        }
        if (changed)
            emit currentProfileChanged(currentProfile());
    }
}

QString QSystemDeviceInfoPrivate::model()
{
    QString name;
    if(productName()== "RX-51")
        return "N900";

    name = "Harmattan"; //fake this for now

    return name;

}

QString QSystemDeviceInfoPrivate::productName()
{
#if !defined(QT_NO_DBUS)
#if defined(Q_WS_MAEMO_6)
    QString dBusService = "com.nokia.SystemInfo";
#else
    /* Maemo 5 */
    QString dBusService = "com.nokia.SystemInfo";
#endif
    QDBusInterface connectionInterface(dBusService,
                                       "/com/nokia/SystemInfo",
                                       "com.nokia.SystemInfo",
                                       QDBusConnection::systemBus());

    QDBusReply< QByteArray > reply = connectionInterface.call("GetConfigValue","/component/product");
    return reply.value();
#endif
}

#endif

//////////////
///////
QSystemScreenSaverPrivate::QSystemScreenSaverPrivate(QObject *parent)
        : QSystemScreenSaverLinuxCommonPrivate(parent),isInhibited(0)
{
    ssTimer = new QTimer(this);
#if !defined(QT_NO_DBUS)
    mceConnectionInterface = new QDBusInterface("com.nokia.mce",
                                                "/com/nokia/mce/request",
                                                "com.nokia.mce.request",
                                                QDBusConnection::systemBus());
#endif
}

QSystemScreenSaverPrivate::~QSystemScreenSaverPrivate()
{
    if (ssTimer->isActive()) {
        ssTimer->stop();
    }
#if !defined(QT_NO_DBUS)
    delete mceConnectionInterface, mceConnectionInterface = 0;
#endif
}

bool QSystemScreenSaverPrivate::setScreenSaverInhibit()
{
    wakeUpDisplay();
    if (!ssTimer->isActive()) {
        connect(ssTimer, SIGNAL(timeout()), this, SLOT(wakeUpDisplay()));
        // Set a wake up interval of 30 seconds.
        // The reason for this is to avoid the situation where
        // a crashed/hung application keeps the display on.
        ssTimer->start(30000);
        isInhibited = true;
    } else {
        isInhibited = false;
    }
     return screenSaverInhibited();
}

void QSystemScreenSaverPrivate::wakeUpDisplay()
{
#if !defined(QT_NO_DBUS)
    if (mceConnectionInterface->isValid()) {
        mceConnectionInterface->call("req_tklock_mode_change", "unlocked");
        mceConnectionInterface->call("req_display_blanking_pause");
    }
#endif
}

bool QSystemScreenSaverPrivate::screenSaverInhibited()
{
    bool displayOn = false;
    GConfItem screenBlankItem("/system/osso/dsm/display/inhibit_blank_mode");
    /* 0 - no inhibit
       1 - inhibit dim with charger
       2 - inhibit blank with charger (display still dims)
       3 - inhibit dim (always)
       4 - inhibit blank (always; display still dims)
*/
    int blankingItem = screenBlankItem.value().toInt();

    bool isBlankingInhibited = false;
    QSystemDeviceInfo devInfo(this);
    QSystemDeviceInfo::PowerState batState = devInfo.currentPowerState();

    if( ((batState == QSystemDeviceInfo::WallPower || batState == QSystemDeviceInfo::WallPowerChargingBattery)
       && blankingItem == 2) || blankingItem == 4) {
        isBlankingInhibited = true;
    }

#if !defined(QT_NO_DBUS)
    if (mceConnectionInterface->isValid()) {
        // The most educated guess for the screen saver being inhibited is to determine
        // whether the display is on. That is because the QSystemScreenSaver cannot
        // prevent other processes from blanking the screen (like, if
        // MCE decides to blank the screen for some reason).
        // but that means it reports to be inhibited when display is on. meaning
        // effectly always inhibited by default. so we try a bit harder
        QDBusReply<QString> reply = mceConnectionInterface->call("get_display_status");
        displayOn = ("on" == reply.value());
    }
#endif
    return (displayOn && isBlankingInhibited && isInhibited);
}

#include "moc_qsysteminfo_maemo_p.cpp"

QTM_END_NAMESPACE
