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

KcmGrub2::KcmGrub2(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, metaData, args)
    , m_data(new GrubData)
{
    qmlRegisterAnonymousType<GrubData>("", 1);
    qmlRegisterAnonymousType<Entry>("", 1);
    qmlRegisterAnonymousType<Language>("", 1);
    qmlRegisterUncreatableType<GrubData>("org.kde.plasma.kcm.data", 1, 0, "DefaultEntry", QStringLiteral("Only for enums"));
    setButtons(Apply);
}

KcmGrub2::~KcmGrub2()
{
}

GrubData *KcmGrub2::grubData() const
{
    return m_data;
}

bool KcmGrub2::isSaveNeeded() const
{
    return m_data->isDirty();
}
bool KcmGrub2::isDefaults() const
{
    return true;
}

void KcmGrub2::load()
{
    m_data->load();
    Q_EMIT settingsChanged();
}
void KcmGrub2::save()
{
    m_data->save();
}
void KcmGrub2::defaults()
{
}

#include "kcmgrub2.moc"
