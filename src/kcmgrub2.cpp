/*
    SPDX-FileCopyrightText: 2013 Reza Fatahilah Shah <rshah0385@kireihana.com>
    SPDX-FileCopyrightText: 2019 Filip Fila <filipfila.kde@gmail.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kcmgrub2.h"

#include <QApplication>
#include <QDebug>

#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KPluginFactory>

#include <common.h>
#include "config.h"
#include <entry.h>
#include <grubdata.h>


K_PLUGIN_FACTORY_WITH_JSON(KCMTestFactory, "kcm_grub2.json", registerPlugin<KcmGrub2>();)

static QJSValue dataSingleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    auto result = new GrubData();
    return scriptEngine->newQObject(result);
}

KcmGrub2::KcmGrub2(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, metaData, args)
{
    // qmlRegisterAnonymousType<GrubData>("org.kde.kcms.grub2", 1);
    // GrubData grubData;
    // qmlRegisterSingletonInstance("org.kde.kcms.grub2",1, 0,"Data",&grubData);
    qmlRegisterSingletonType("org.kde.kcms.grub2", 1, 0, "Data", dataSingleton);
}

KcmGrub2::~KcmGrub2()
{
}


#include "kcmgrub2.moc"
