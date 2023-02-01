

// Own
#include "helper.h"

// Qt
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QDir>
#include <QtGlobal>

// KDE
#include <KAuth/HelperSupport>
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

ActionReply Helper::save(QVariantMap args)
{
    QDir dir("/testRoot");
    if (dir.exists())
        dir.removeRecursively();

    const QString PATH = args.value("homeDir").toString() + "/.local/share/grub-editor-cpp";

    QString filePath = PATH + "/grubToWrite.txt";
    ActionReply reply;

    if (!QFile::exists(filePath)) {
        reply.addData("result", "failed");
        return reply;
    }
    const QString saveFile = args.value("saveFile").toString();

    if (QFile::exists(saveFile)) {
        QFile::remove(saveFile);
    }
    QString busAddress = args.value("busAddress").toString();

    // Necessary to connect to user's session bus
    seteuid(args.value("euid").value<uid_t>());
    QDBusConnection bus = QDBusConnection::connectToBus(busAddress, "userbus");
    seteuid(0);

    QFile::copy(filePath, saveFile);

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

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmgrub2", Helper)
