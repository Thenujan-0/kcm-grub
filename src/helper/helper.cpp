

// Own
#include "helper.h"

// Qt
#include <QDebug>
#include <QDir>

// KDE
#include <KAuth/HelperSupport>
#include <KLocalizedString>
#include <KProcess>

// Project
#include "../common.h"
#include "../config.h"

// The $PATH environment variable is emptied by D-Bus activation,
// so let's provide a sane default. Needed for os-prober to work.
static const QLatin1String path("/usr/sbin:/usr/bin:/sbin:/bin");

Helper::Helper()
{
    qputenv("PATH", path.latin1());
}

// void Helper::set_value(const Qstring &fileName)
// {
//     QFile file(fileName);
// }

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

    if (QFile::exists("/etc/default/grub")) {
        QFile::remove("/etc/default/grub");
    }

    bool sucess = QFile::copy(filePath, "/etc/default/grub");
    reply.addData("result", sucess);
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmgrub2", Helper)
