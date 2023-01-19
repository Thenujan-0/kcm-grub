/*
    SPDX-FileCopyrightText: 2013 Reza Fatahilah Shah <rshah0385@kireihana.com>

    SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KCMGRUB2_H
#define KCMGRUB2_H

#include <KQuickAddons/ManagedConfigModule>
#include <entry.h>
#include <grubdata.h>

class KcmGrub2 : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(GrubData *grubData READ grubData CONSTANT)
public:
    explicit KcmGrub2(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~KcmGrub2() override;
    GrubData *grubData() const;
    // QJSValue dataSingleton(QQmlEngine *engine, QJSEngine *scriptEngine);

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

private:
    bool isSaveNeeded() const override;
    bool isDefaults() const override;
    GrubData *m_data;
};

#endif // TESTKCM_H
