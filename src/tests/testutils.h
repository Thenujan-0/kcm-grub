#ifndef TESTUTILs_H
#define TESTUTILs_H

#include <optional>

#include "config.h"
#include "grubdata.h"

using std::optional;

QString getDataPath();
QString getCWD();
QString getValue(QString key, const QString &config);
optional<QStringList> lineChanged(const QString &state1, const QString &state2);
optional<QString> lineAdded(const QString &state1, const QString &state2);

QMetaProperty getDefaultEntryTypeProp(GrubData *data);
QMetaProperty getDefaultEntryProp(GrubData *data);
QMetaProperty getTimeoutProp(GrubData *data);
QMetaProperty getLanguageProp(GrubData *data);
QMetaProperty getRecoveryProp(GrubData *data);
#endif