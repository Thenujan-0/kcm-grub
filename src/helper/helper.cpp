

// Own
#include "helper.h"

// Qt
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QtGlobal>

// KDE
#include <KAuth/HelperSupport>
#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KProcess>

// Project
#include "../common.h"
#include "../config.h"

#include <unistd.h>
// The $PATH environment variable is emptied by D-Bus activation,
// so let's provide a sane default. Needed for os-prober to work.
static const QLatin1String path("/usr/sbin:/usr/bin:/sbin:/bin");

Helper::Helper()
{
    qputenv("PATH", path.latin1());
}

ActionReply Helper::executeCommand(const QStringList &command)
{
    KProcess process;
    process.setProgram(command);
    process.setOutputChannelMode(KProcess::MergedChannels);

    qDebug() << "Executing" << command.join(QLatin1String(" "));
    int exitCode = process.execute();
    const QByteArray output = process.readAll();

    ActionReply reply;
    if (exitCode != 0) {
        reply = ActionReply::HelperErrorReply(exitCode);
        QString errorMessage;
        switch (exitCode) {
        case -2:
            errorMessage = i18nc("@info", "The process could not be started.");
            break;
        case -1:
            errorMessage = i18nc("@info", "The process crashed.");
            break;
        default:
            errorMessage = QString::fromUtf8(output);
            break;
        }
        // reply.setErrorDescription(i18nc("@info", "Command: <command>%1</command><nl/>", command.join(QLatin1String(" "))) +
        //                           errorDescription(exitCode, errorMessage));
    } else {
        reply.addData(QStringLiteral("command"), command);
        reply.addData(QStringLiteral("output"), output);
    }
    return reply;
}

ActionReply Helper::load(QVariantMap args)
{
    ActionReply reply;
    LoadOperations operations = (LoadOperations)(args.value(QStringLiteral("operations")).toInt());

    if (operations.testFlag(MenuFile)) {
        QFile file(grubMenuPath());
        bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
        reply.addData(QStringLiteral("menuSuccess"), ok);
        if (ok) {
            reply.addData(QStringLiteral("menuContents"), file.readAll());
        } else {
            reply.addData(QStringLiteral("menuError"), file.error());
            reply.addData(QStringLiteral("menuErrorString"), file.errorString());
        }
    }
    if (operations.testFlag(ConfigurationFile)) {
        QFile file(grubConfigPath());
        bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
        reply.addData(QStringLiteral("configSuccess"), ok);
        if (ok) {
            reply.addData(QStringLiteral("configContents"), file.readAll());
        } else {
            reply.addData(QStringLiteral("configError"), file.error());
            reply.addData(QStringLiteral("configErrorString"), file.errorString());
        }
    }
    if (operations.testFlag(EnvironmentFile)) {
        QFile file(grubEnvPath());
        bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
        reply.addData(QStringLiteral("envSuccess"), ok);
        if (ok) {
            reply.addData(QStringLiteral("envContents"), file.readAll());
        } else {
            reply.addData(QStringLiteral("envError"), file.error());
            reply.addData(QStringLiteral("envErrorString"), file.errorString());
        }
    }
    if (operations.testFlag(MemtestFile)) {
        bool memtest = QFile::exists(grubMemtestPath());
        reply.addData(QStringLiteral("memtest"), memtest);
        if (memtest) {
            reply.addData(QStringLiteral("memtestOn"), (bool)(QFile::permissions(grubMemtestPath()) & (QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther)));
        }
    }

    if (operations.testFlag(Locales)) {
        reply.addData(QStringLiteral("locales"),
                      QDir(grubLocalePath())
                          .entryList(QStringList() << QStringLiteral("*.mo"), QDir::Files)
                          .replaceInStrings(QRegExp(QLatin1String("\\.mo$")), QString()));
    }
    return reply;
}

ActionReply Helper::save(QVariantMap args)
{
    QDir dir("/testRoot");
    if (dir.exists())
        dir.removeRecursively();

    QString filePath = args.value("sourceFilePath").toString();
    ActionReply reply;

    if (!QFile::exists(filePath)) {
        reply.addData("result", "failed");
        return reply;
    }
    const QString saveFile = args.value("saveFile").toString();

    if (args.value("rootCopyNeeded").toBool()) {
        copyTo(filePath, saveFile);
    }

    QString busAddress = args.value("busAddress").toString();

    // Necessary to connect to user's session bus
    seteuid(args.value("euid").value<uid_t>());
    QDBusConnection bus = QDBusConnection::connectToBus(busAddress, "userbus");
    seteuid(0);

    QDBusMessage m = QDBusMessage::createMethodCall("org.kde.kcontrol.kcmgrub", "/internal", "", "emitSavingStarted");
    bus.send(m);

    KProcess *process = new KProcess();
    QString cmd("grub-mkconfig -o /boot/grub/grub.cfg");
    process->setShellCommand(cmd);
    process->setOutputChannelMode(KProcess::MergedChannels);

    process->start();
    connect(process, &QProcess::readyReadStandardOutput, this, [bus, process, &reply]() {
        const QByteArray output = process->readAll();
        QDBusMessage m = QDBusMessage::createMethodCall("org.kde.kcontrol.kcmgrub", "/internal", "", "updateCommandOutput");
        QList<QVariant> args;
        args.append(QString::fromUtf8(output));
        m.setArguments(args);
        bus.send(m);
    });
    process->waitForFinished(-1);
    reply.addData("exitCode", process->exitCode());
    return reply;
}

ActionReply Helper::changemenupermissions(QVariantMap args)
{
    ActionReply reply;
    QString filePath = args.value("filePath").toString();
    QFile file{filePath};
    file.setPermissions(file.permissions() | QFile::ReadOther);
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmgrub", Helper)
