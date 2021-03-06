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

#include <qfeedbackactuator.h>
#include "qfeedbacktestplugin.h"
#include <QtCore/QtPlugin>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QVariant>
#include <QDebug>

Q_EXPORT_PLUGIN2(feedback_testplugin, QFeedbackTestPlugin)

        QFeedbackTestPlugin::QFeedbackTestPlugin() : QObject(qApp), mHapticState(QFeedbackEffect::Stopped), mFileState(QFeedbackEffect::Stopped)
{
    actuators_ << createFeedbackActuator(this, 0) << createFeedbackActuator(this, 1);
    mHapticTimer.setSingleShot(true);
    connect(&mHapticTimer, SIGNAL(timeout()), this, SLOT(timerExpired()));
}

QFeedbackTestPlugin::~QFeedbackTestPlugin()
{
}

QFeedbackInterface::PluginPriority QFeedbackTestPlugin::pluginPriority()
{
    return PluginHighPriority; // to make sure we get used
}

QList<QFeedbackActuator*> QFeedbackTestPlugin::actuators()
{
    return actuators_;
}

void QFeedbackTestPlugin::setActuatorProperty(const QFeedbackActuator &actuator, ActuatorProperty prop, const QVariant &value)
{
    Q_UNUSED(actuator)
    Q_UNUSED(prop)
    Q_UNUSED(value)
}

QVariant QFeedbackTestPlugin::actuatorProperty(const QFeedbackActuator &actuator, ActuatorProperty prop)
{
    Q_UNUSED(actuator)

    switch (prop) {
        case Name:
        if (actuator.id() == 0)
            return QString(QLatin1String("test plugin"));
        else
            return QString(QLatin1String("5555"));

        case State:
            return static_cast<int>(QFeedbackActuator::Unknown);

        case Enabled:
            return false;
        default:
            break;
    }

    return QVariant();
}

bool QFeedbackTestPlugin::isActuatorCapabilitySupported(const QFeedbackActuator &, QFeedbackActuator::Capability cap)
{
    switch (cap) {
        case QFeedbackActuator::Envelope:
        case QFeedbackActuator::Period:
            return true;
        default:
            break;
    }

    return false;
}


void QFeedbackTestPlugin::updateEffectProperty(const QFeedbackHapticsEffect *effect, EffectProperty ep)
{
    if (ep == QFeedbackHapticsInterface::Duration)
        mHapticTimer.setInterval(effect->duration());
}

void QFeedbackTestPlugin::setEffectState(const QFeedbackHapticsEffect *effect, QFeedbackEffect::State state)
{
    Q_UNUSED(effect)
    if (mHapticState != state) {
        mHapticState = state;
        if (mHapticState == QFeedbackEffect::Running) {
            mHapticTimer.start();
        } else if (mHapticState == QFeedbackEffect::Stopped) {
            mHapticTimer.stop();
        } else if (mHapticState == QFeedbackEffect::Paused) {
            // In theory should set the duration to the remainder...
            mHapticTimer.stop();
        }
    }
}

QFeedbackEffect::State QFeedbackTestPlugin::effectState(const QFeedbackHapticsEffect *effect)
{
    Q_UNUSED(effect)
    return mHapticState;
}

void QFeedbackTestPlugin::timerExpired()
{
    mHapticState = QFeedbackEffect::Stopped;
}



void QFeedbackTestPlugin::setLoaded(QFeedbackFileEffect *effect, bool load)
{
    Q_UNUSED(effect)
    if (effect->source() == QUrl("load")) {
        // Succeed the load
        if (load) {
            mFileState = QFeedbackEffect::Loading;
            reportLoadFinished(effect, true); // not strictly true
        } else
            mFileState = QFeedbackEffect::Stopped;
    } else {
        // Fail the load
        if (load)
            reportLoadFinished(effect, false);
    }
}

void QFeedbackTestPlugin::setEffectState(QFeedbackFileEffect *effect, QFeedbackEffect::State state)
{
    Q_UNUSED(effect)
    mFileState = state;
}

QFeedbackEffect::State QFeedbackTestPlugin::effectState(const QFeedbackFileEffect *effect)
{
    Q_UNUSED(effect)
    return mFileState;
}

int QFeedbackTestPlugin::effectDuration(const QFeedbackFileEffect *effect)
{
    Q_UNUSED(effect)
    return 5678;
}

QStringList QFeedbackTestPlugin::supportedMimeTypes()
{
    return QStringList() << "x-test/this is a test";
}

bool QFeedbackTestPlugin::play(QFeedbackEffect::ThemeEffect themeEffect)
{
    if (themeEffect == QFeedbackEffect::ThemeBasic)
        return true;
    else
        return false;
}
