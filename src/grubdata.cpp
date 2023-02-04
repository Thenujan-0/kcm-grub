
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QTimer>
#include <QtGlobal>

#include <entry.h>
#include <grubdata.h>
#include <common.h>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KIO/CopyJob>
#include <KJob>

#include <unistd.h>

QString HOME = qgetenv("HOME");

static const QString TEMPDATAFILE = HOME + QString("/.local/share/grub-editor-cpp/grubToWrite.txt");

GrubData::GrubData(QObject *parent)
    :m_issues(QStringList()),
    m_currFileName(QString("/etc/default/grub"))
{
    Q_UNUSED(parent);

    QDBusConnection::sessionBus().registerService("org.kde.kcontrol.kcmgrub");
    QDBusConnection::sessionBus().registerObject("/internal", this, QDBusConnection::ExportAllSlots);
    readAll();
}

void GrubData::load()
{
    readAll();
}

bool GrubData::updateCommandOutput(QString text)
{
    Q_EMIT updateOutput(text);
    return true;
}

void GrubData::emitSavingStarted()
{
    Q_EMIT savingStarted();
}

QString GrubData::parseTitle(const QString &line){
    QChar ch;
    QString entry, lineStr = line;
    QTextStream stream(&lineStr, QIODevice::ReadOnly | QIODevice::Text);

    stream.skipWhiteSpace();
    if (stream.atEnd()) {
        return QString();
    }

    stream >> ch;
    entry += ch;
    if (ch == QLatin1Char('\'')) {
        do {
            if (stream.atEnd()) {
                return QString();
            }
            stream >> ch;
            entry += ch;
        } while (ch != QLatin1Char('\''));
    } else if (ch == QLatin1Char('"')) {
        do {
            if (stream.atEnd()) {
                return QString();
            }
            stream >> ch;
            entry += ch;
        } while (ch != QLatin1Char('"') || entry.at(entry.size() - 2) == QLatin1Char('\\'));
    } else {
        do {
            if (stream.atEnd()) {
                return QString();
            }
            stream >> ch;
            entry += ch;
        } while (!ch.isSpace() || entry.at(entry.size() - 2) == QLatin1Char('\\'));
        entry.chop(1); //remove trailing space
    }
    return entry;
}

void GrubData::deleteEntries()
{
    for (const Entry *osEntry : qAsConst(m_osEntries)) {
        delete osEntry;
    }
    m_osEntries.clear();
}

void GrubData::parseEntries(const QString &config){
    bool inEntry = false;
    int menuLvl = 0;
    QList<int> levelCount;
    levelCount.append(0);
    QList<Entry::Title> submenus;
    QString word, configStr = config;
    QTextStream stream(&configStr, QIODevice::ReadOnly | QIODevice::Text);

    deleteEntries();
    while (!stream.atEnd()) {
        //Read the first word of the line
        stream >> word;
        if (stream.atEnd()) {
            return;
        }
        //If the first word is known, process the rest of the line
        if (word == QLatin1String("menuentry")) {
            if (inEntry) {
                qCritical() << "Malformed configuration file! Aborting entries' parsing.";
                qDebug() << "A 'menuentry' directive was detected inside the scope of a menuentry.";
                deleteEntries();
                return;
            }
            Entry *entry = new Entry(parseTitle(stream.readLine()), levelCount.at(menuLvl), Entry::Menuentry, menuLvl);
            if (menuLvl > 0) {
                entry->setAncestors(submenus);
            }
            m_osEntries.append(entry);
            levelCount[menuLvl]++;
            inEntry = true;
            continue;
        } else if (word == QLatin1String("submenu")) {
            if (inEntry) {
                qCritical() << "Malformed configuration file! Aborting entries' parsing.";
                qDebug() << "A 'submenu' directive was detected inside the scope of a menuentry.";
                deleteEntries();
                return;
            }
            Entry *entry = new Entry(parseTitle(stream.readLine()), levelCount.at(menuLvl), Entry::Submenu, menuLvl);
            if (menuLvl > 0) {
                entry->setAncestors(submenus);
            }
            m_osEntries.append(entry);
            submenus.append(entry->title());
            levelCount[menuLvl]++;
            levelCount.append(0);
            menuLvl++;
            continue;
        } else if (word == QLatin1String("linux")) {
            if (!inEntry) {
                qCritical() << "Malformed configuration file! Aborting entries' parsing.";
                qDebug() << "A 'linux' directive was detected outside the scope of a menuentry.";
                deleteEntries();
                return;
            }
            stream >> word;
            m_osEntries.last()->setKernel(word);
        } else if (word == QLatin1String("}")) {
            if (inEntry) {
                inEntry = false;
            } else if (menuLvl > 0) {
                submenus.removeLast();
                levelCount[menuLvl] = 0;
                menuLvl--;
            }
        }
        //Drop the rest of the line
        stream.readLine();
    }
}

void GrubData::parseSettings(const QString &config)
{
    // qWarning("parse settings called again");
    QString line, configStr = config;
    QTextStream stream(&configStr, QIODevice::ReadOnly | QIODevice::Text);

    m_settings.clear();
    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();
        if (line.contains(QRegExp(QLatin1String("^(GRUB_|LANGUAGE=)")))) {
            m_settings[line.section(QLatin1Char('='), 0, 0)] = line.section(QLatin1Char('='), 1);
        }
    }
    addDefaultValues();
    parseValues();
}

void GrubData::addDefaultValues()
{
    QMap<QString, QString> map;
    map["GRUB_TIMEOUT"] = "5";
    map["GRUB_DEFAULT"] = "0";
    map["GRUB_DISABLE_OS_PROBER"] = "true";
    map["GRUB_TIMEOUT_STYLE"] = "menu";

    QMapIterator<QString, QString> i(map);
    while (i.hasNext()) {
        i.next();
        if (m_settings.value(i.key()).isEmpty()) {
            m_settings[i.key()] = i.value();
        }
    }

    // Add the values that will be used by grub because of 30_os-prober override
    if (unquoteWord(m_settings["GRUB_DISABLE_OS_PROBER"]) == "false") {
        if (unquoteWord(m_settings["GRUB_TIMEOUT"]) == "0") {
            m_settings["GRUB_TIMEOUT"] = "10";
        }
        m_settings["GRUB_TIMEOUT_STYLE"] = "menu";
    }
}

Entry *GrubData::findEntry(const QString &value)
{
    QList<Entry *>::iterator i;
    for (i = m_osEntries.begin(); i != m_osEntries.end(); ++i) {
        Entry *entry = *i;
        if (entry->fullTitle() == value || entry->fullNumTitle() == value) {
            return entry;
        }
    }
    // If entry wasn't found then, grub will use the default falue for GRUB_DEFAULT therefore,
    qWarning() << "Invalid value for GRUB_DEFAULT";
    i = m_osEntries.begin();
    Entry *entry = *i;

    return entry;
}

void GrubData::parseValues()
{
    m_timeoutStyle_orig = unquoteWord(m_settings.value("GRUB_TIMEOUT_STYLE"));
    m_timeout_orig = unquoteWord(m_settings.value("GRUB_TIMEOUT")).toFloat();

    QString val = unquoteWord(m_settings.value("GRUB_DEFAULT"));

    if (val == "saved") {
        m_defaultEntryType_orig = GrubData::DefaultEntryType::PreviouslyBooted;
    } else {
        m_defaultEntry_orig = findEntry(val);
        m_defaultEntryType_orig = GrubData::DefaultEntryType::Predefined;
    }

    if (m_settings.contains("GRUB_HIDDEN_TIMEOUT")) {
        qWarning() << "Use of deprecated GRUB_HIDDEN_TIMEOUT";
    }

    m_lookForOtherOs_orig = !(unquoteWord(m_settings["GRUB_DISABLE_OS_PROBER"]) == "true");

    m_timeout = m_timeout_orig;
    m_timeoutStyle = m_timeoutStyle_orig;
    m_defaultEntryType = m_defaultEntryType_orig;
    m_lookForOtherOs = m_lookForOtherOs_orig;
    m_defaultEntry = m_defaultEntry_orig;
}

void GrubData::save()
{
    initCache();
    if (m_defaultEntryType != m_defaultEntryType_orig) {
        if (m_defaultEntryType == DefaultEntryType::PreviouslyBooted) {
            setValue("GRUB_DEFAULT", "saved");
            // TODO check if grub_saved_default is needed here
        } else {
            // qWarning() << m_defaultEntry;
            setValue("GRUB_DEFAULT", quoteWord(m_defaultEntry->fullNumTitle()));
        }
    } else if (m_defaultEntryType_orig == DefaultEntryType::Predefined && m_defaultEntry != m_defaultEntry_orig) {
        // qWarning() << m_defaultEntry;
        setValue("GRUB_DEFAULT", quoteWord(m_defaultEntry->fullNumTitle()));
    }

    if (m_timeout != m_timeout_orig) {
        setValue("GRUB_TIMEOUT", quoteWord(QString::number(m_timeout)));
    }

    if (m_lookForOtherOs != m_lookForOtherOs_orig) {
        const QString value = m_lookForOtherOs ? "false" : "true";
        setValue("GRUB_DISABLE_OS_PROBER", value);
    }
    if (m_timeoutStyle != m_timeoutStyle_orig) {
        setValue("GRUB_TIMEOUT_STYLE", m_timeoutStyle);
    }

    KAuth::Action saveAction("org.kde.kcontrol.kcmgrub2.save");
    saveAction.setHelperId("org.kde.kcontrol.kcmgrub2");
    saveAction.addArgument("homeDir", qgetenv("HOME"));
    saveAction.addArgument("saveFile", m_currFileName);
    saveAction.addArgument("euid", geteuid());
    saveAction.addArgument("busAddress", qgetenv("DBUS_SESSION_BUS_ADDRESS"));
    KAuth::ExecuteJob *job = saveAction.execute();
    job->start();
    connect(job, &KAuth::ExecuteJob::result, this, [this, job]() {
        qWarning() << "job finished";
        QString contents = job->data()["fakeresult"].toString();
        QString contents2 = job->data()["exitCode"].toString();
        qWarning() << "KAuth result:" << contents;
        qWarning() << "KAuth exitCode:" << contents2;
        Q_EMIT savingFinished();
        readAll();
    });
}

bool GrubData::setValue(QString key, QString val, QString readFileName)
{
    if (readFileName == "") {
        readFileName = TEMPDATAFILE;
    }
    QFile readFile(readFileName);
    // qWarning() << readFileName;
    QString toWrite;
    bool replaced = false;
    if (!readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&readFile);

    if (key == QString("GRUB_DEFAULT")) {
        val.replace(QString(" >"), QString(">"));
    }

    while (!in.atEnd()) {
        QString line = in.readLine();

        QString originalLine = line;
        // remove all empty spaces
        line.remove(QChar(' '));

        if (line.startsWith(QChar('#') + key)) {
            toWrite += key + QChar('=') + val + QChar('\n');
            replaced = true;
        } else if (line.startsWith('#')) {
            toWrite += originalLine + QChar('\n');
            continue;

        } else if (line.startsWith(key + QChar('=')) && !replaced) {
            line = key + QChar('=') + val;
            toWrite += line + QChar('\n');
            replaced = true;
            continue;
        } else {
            toWrite += line + QChar('\n');
        }
    }
    if (!replaced) {
        toWrite += key + QChar('=') + val + QChar('\n');
    }
    readFile.close();
    if (!readFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&readFile);
    out << toWrite;
    readFile.close();
    return true;
}

void GrubData::initCache()
{
    const QString PATH = qgetenv("HOME") + "/.local/share/grub-editor-cpp";
    const QDir dataDir(PATH);
    if (!dataDir.exists())
        dataDir.mkpath(PATH);

    QString filePath = PATH + "/grubToWrite.txt";
    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }

    QFile::copy(m_currFileName, filePath);

    // qWarning() << PATH;
}

void GrubData::setCurrentFile(const QString &fileName)
{
    m_currFileName = fileName;
    QByteArray fileContents;
    readAll();
}

QString GrubData::getValue(const QString &key)
{
    return m_settings[key];
}

bool GrubData::isDirty()
{
    return (m_timeout != m_timeout_orig) || (m_lookForOtherOs != m_lookForOtherOs_orig) || (m_defaultEntryType != m_defaultEntryType_orig)
        || (m_defaultEntry->fullTitle() != m_defaultEntry_orig->fullTitle());
}

void GrubData::readAll(){
    optional<QString> fileContents;
    LoadOperations operations = NoOperation;
    // qWarning() << "File contents are being read";

    fileContents = readFile("/boot/grub/grub.cfg");
    if (fileContents.has_value()) {
        parseEntries(fileContents.value());
    } else {
        operations |= MenuFile;
    }
    fileContents = readFile(m_currFileName);
    if (fileContents.has_value()) {
        parseSettings(fileContents.value());
    } else {
        operations |= ConfigurationFile;
    }
    // if (readFile(grubEnvPath(), fileContents)) {
    //     parseEnv(QString::fromUtf8(fileContents.constData()));
    // } else {
    //     operations |= EnvironmentFile;
    // }
    // if (QFile::exists(grubMemtestPath())) {
    //     m_memtest = true;
    //     m_memtestOn = (bool)(QFile::permissions(grubMemtestPath()) & (QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther));
    // } else {
    //     operations |= MemtestFile;
    // }

    // if (QFileInfo(grubLocalePath()).isReadable()) {
    //     m_locales = QDir(grubLocalePath()).entryList(QStringList() << QStringLiteral("*.mo"), QDir::Files).replaceInStrings(QRegExp(QLatin1String("\\.mo$")), QString());
    // } else {
    //     operations |= Locales;
    // }
    Q_EMIT osEntriesChanged();
    Q_EMIT dataChanged();
}
