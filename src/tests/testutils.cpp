
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QtGlobal>

#include "testutils.h"
#include <filesystem>
#include <optional>

using std::optional;

QString getCWD()
{
    QString cwd = QString::fromStdString(std::filesystem::current_path().parent_path().string());
    return cwd;
}

QString getTestFilesDir()
{
    const QString filePath = getCWD() + "/src/tests/testCaseFiles";
    return filePath;
}

QString getDataPath()
{
    const QString PATH = qgetenv("HOME") + "/.local/share/grub-editor-cpp";

    return PATH;
}

QString getValue(QString key, const QString &config)
{
    QString value;
    QFile confFile(config);
    if (!confFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "";
    }
    QTextStream stream(&confFile);
    QString line;

    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();
        const QString pattern = QLatin1String("^(") + key + QLatin1String("=)");
        if (line.contains(QRegExp(pattern))) {
            value = line.section(QLatin1Char('='), 1);
            return value;
        }
    }
    return "";
}

// This function is only for the state of /etc/default/grub. It should only be used compare states after one use of stateValue
// in other words, only one change should exist
// Returns QStringList where first item is removed line and the second item is added line
optional<QStringList> lineChanged(const QString &state1, const QString &state2)
{
    QStringList lines1 = state1.split('\n');
    QStringList lines2 = state2.split('\n');
    if (lines1.count() < lines2.count()) {
        if (lines2.last() != "") {
            return {};
        }
        qWarning() << "state1 has less lines than state2. Lines count in state1, state2 are " << lines1.count() << lines2.count();
    }
    QStringList toReturn;
    for (int i = 0; i < lines1.count(); i++) {
        if (!(lines1[i] == lines2[i])) {
            if (toReturn.isEmpty()) {
                toReturn << lines1[i] << lines2[i];
            } else {
                // More than one change
                qWarning() << "More than one change";
                return {};
            }
        }
    }

    if (!toReturn.isEmpty()) {
        qWarning() << toReturn;
        return toReturn;
    }
    qWarning() << "Couldn't find different lines";
    return {};
}

optional<QString> lineAdded(const QString &state1, const QString &state2)
{
    QStringList lines1 = state1.split('\n');
    QStringList lines2 = state2.split('\n');

    bool onlyOneLineAdded = (lines1.count() + 1 == lines2.count());
    bool oneLineAndNewLineAdded = (lines1.count() + 2 == lines2.count()) && lines2.last() == "" && lines1.last() != "";
    qWarning() << oneLineAndNewLineAdded;
    qWarning() << lines2.last() << lines1.last();
    if (!(onlyOneLineAdded || oneLineAndNewLineAdded)) {
        qWarning() << "Line change is not +1 line. Lines count of state1, state2 " << lines1.count() << lines2.count();
        return {};
    }

    for (int i = 0; i < lines1.count(); i++) {
        if (!(lines1[i] == lines2[i])) {
            qWarning() << "A line that exists in state1 doesn't exist in state2";
            return {};
        }
    }

    if (onlyOneLineAdded) {
        return lines2.last();
    } else {
        return lines2[lines2.count() - 2];
    }
}