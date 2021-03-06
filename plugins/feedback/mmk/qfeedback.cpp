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

#include <QtCore/QtPlugin>
#include <QtCore/QCoreApplication>

#include <QDebug>
#include "qfeedback.h"

Q_EXPORT_PLUGIN2(feedback_mmk, QFeedbackMMK)

QFeedbackMMK::QFeedbackMMK() : QObject(qApp)
{
}

QFeedbackMMK::~QFeedbackMMK()
{
    foreach(FeedbackInfo fi, mEffects.values()) {
        delete fi.soundEffect;
    }
}

void QFeedbackMMK::setLoaded(QFeedbackFileEffect *effect, bool load)
{
    if (!effect)
        return;

    // See if we have seen this effect before
    FeedbackInfo fi = mEffects.value(effect);

    if (load) {
        // Well.. we might already have an effect, since we don't create them until
        // we load...
        if (fi.loaded) {
            // We've already loaded?
            return;
        } else {
            if (fi.soundEffect) {
                // We've started a load, they must just be impatient
                // Pushing this elevator button does nothing..
                return;
            } else {
                // New sound effect!
                fi.soundEffect = new QSoundEffect(this);
                connect(fi.soundEffect, SIGNAL(loadedChanged()), this, SLOT(soundEffectLoaded()));
                fi.soundEffect->setSource(effect->source());
                mEffects.insert(effect, fi);
                mEffectMap.insert(fi.soundEffect, effect);
            }
        }
    } else {
        // Time to unload.
        if (fi.soundEffect) {
            mEffectMap.remove(fi.soundEffect);
            delete fi.soundEffect;
        }
        mEffects.remove(effect);
    }
}

void QFeedbackMMK::setEffectState(QFeedbackFileEffect *effect, QFeedbackEffect::State state)
{
    FeedbackInfo fi = mEffects.value(effect);
    switch (state)
    {
        case QFeedbackEffect::Stopped:
            if (fi.playing) {
                Q_ASSERT(fi.soundEffect);
                fi.soundEffect->stop();
                fi.playing = false;
            }
            break;

        case QFeedbackEffect::Paused:
            // Well, we can't pause, really
            reportError(effect, QFeedbackEffect::UnknownError);
            break;

        case QFeedbackEffect::Running:
            if (fi.playing) {
                // We're already playing.
            } else if (fi.soundEffect){
                fi.soundEffect->play();
                fi.playing = true;
            }
            break;
        default:
            break;
    }
}

QFeedbackEffect::State QFeedbackMMK::effectState(const QFeedbackFileEffect *effect)
{
    FeedbackInfo fi = mEffects.value(effect);

    if (fi.soundEffect) {
        if (fi.playing)
            return QFeedbackEffect::Running;
        if (fi.loaded)
            return QFeedbackEffect::Stopped; // No idle?
        return QFeedbackEffect::Loading;
    }
    return QFeedbackEffect::Stopped;
}

int QFeedbackMMK::effectDuration(const QFeedbackFileEffect *effect)
{
    Q_UNUSED(effect);
    // XXX This isn't supported by MMK currently
    return 0;
}

QStringList QFeedbackMMK::supportedMimeTypes()
{
    return QSoundEffect::supportedMimeTypes();
}

void QFeedbackMMK::soundEffectLoaded()
{
    QSoundEffect* se = qobject_cast<QSoundEffect*>(sender());
    if (se) {
        // Hmm, now look up the right sound effect
        QFeedbackFileEffect* fe = mEffectMap.value(se);

        if (fe) {
            reportLoadFinished(fe, se->isLoaded());
        }
    }
}
