/*
    SPDX-FileCopyrightText: 2013 Reza Fatahilah Shah <rshah0385@kireihana.com>
    SPDX-FileCopyrightText: 2019 Filip Fila <filipfila.kde@gmail.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kcmgrub.h"

#include <QApplication>
#include <QDebug>

#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KPluginFactory>

#include "config.h"
#include <common.h>
#include <entry.h>
#include <grubdata.h>

K_PLUGIN_FACTORY_WITH_JSON(KCMTestFactory, "kcm_grub.json", registerPlugin<KcmGrub>();)

KcmGrub::KcmGrub(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, metaData, args)
    , m_data(new GrubData)
{
    qmlRegisterAnonymousType<GrubData>("", 1);
    qmlRegisterAnonymousType<Entry>("", 1);
    qmlRegisterAnonymousType<Language>("", 1);
    qRegisterMetaType<Resolution>();
    qmlRegisterUncreatableType<GrubData>("org.kde.plasma.kcm.data", 1, 0, "DefaultEntry", QStringLiteral("Only for enums"));
    setButtons(Apply);
}

KcmGrub::~KcmGrub()
{
}

GrubData *KcmGrub::grubData() const
{
    return m_data;
}

bool KcmGrub::isSaveNeeded() const
{
    return m_data->isDirty();
}
bool KcmGrub::isDefaults() const
{
    return true;
}

void KcmGrub::load()
{
    m_data->load();
    Q_EMIT settingsChanged();
}
void KcmGrub::save()
{
    m_data->save();
}
void KcmGrub::defaults()
{
}

#include "kcmgrub.moc"
