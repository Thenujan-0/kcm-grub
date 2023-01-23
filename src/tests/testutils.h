
#ifndef TESTUTILs_H
#define TESTUTILs_H

#include <optional>
using std::optional;

QString getDataPath();
QString getCWD();
QString getTestFilesDir();
QString getValue(QString key, const QString &config);
optional<QStringList> lineChanged(const QString &state1, const QString &state2);
optional<QString> lineAdded(const QString &state1, const QString &state2);
#endif