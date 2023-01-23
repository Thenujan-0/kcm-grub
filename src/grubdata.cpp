
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

QString HOME = qgetenv("HOME");

static const QString TEMPDATAFILE = HOME + QString("/.local/share/grub-editor-cpp/grubToWrite.txt");

GrubData::GrubData(QObject *parent)
    :m_issues(QStringList()),
    m_currFileName(QString("/etc/default/grub"))
{
    readAll();
    // qWarning()<<"loaded everything here";
    // for (const Entry &osEntry :qAsConst(m_osEntries)){
    //     qWarning() <<osEntry.fullTitle();
    // }
    // qWarning()<<m_osEntries;
}

bool GrubData::readFile(const QString &fileName, QByteArray &fileContents){
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for reading:" << fileName;
        qDebug() << "Error code:" << file.error();
        qDebug() << "Error description:" << file.errorString();
        qDebug() << "The helper will now attempt to read this file.";
        return false;
    }
    fileContents = file.readAll();
    return true;
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

void GrubData::parseEntries(const QString &config){
    bool inEntry = false;
    int menuLvl = 0;
    QList<int> levelCount;
    levelCount.append(0);
    QList<Entry::Title> submenus;
    QString word, configStr = config;
    QTextStream stream(&configStr, QIODevice::ReadOnly | QIODevice::Text);

    m_osEntries.clear();
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
                m_osEntries.clear();
                return;
            }
            Entry entry(parseTitle(stream.readLine()), levelCount.at(menuLvl), Entry::Menuentry, menuLvl);
            if (menuLvl > 0) {
                entry.setAncestors(submenus);
            }
            m_osEntries.append(entry);
            levelCount[menuLvl]++;
            inEntry = true;
            continue;
        } else if (word == QLatin1String("submenu")) {
            if (inEntry) {
                qCritical() << "Malformed configuration file! Aborting entries' parsing.";
                qDebug() << "A 'submenu' directive was detected inside the scope of a menuentry.";
                m_osEntries.clear();
                return;
            }
            Entry entry(parseTitle(stream.readLine()), levelCount.at(menuLvl), Entry::Submenu, menuLvl);
            if (menuLvl > 0) {
                entry.setAncestors(submenus);
            }
            m_osEntries.append(entry);
            submenus.append(entry.title());
            levelCount[menuLvl]++;
            levelCount.append(0);
            menuLvl++;
            continue;
        } else if (word == QLatin1String("linux")) {
            if (!inEntry) {
                qCritical() << "Malformed configuration file! Aborting entries' parsing.";
                qDebug() << "A 'linux' directive was detected outside the scope of a menuentry.";
                m_osEntries.clear();
                return;
            }
            stream >> word;
            m_osEntries.last().setKernel(word);
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
}
void GrubData::parseValues()
{
    m_timeout_orig = unquoteWord(m_settings.value("GRUB_TIMEOUT")).toFloat();
    // qWarning() << m_timeout_orig << "timeout" << unquoteWord(m_settings["GRUB_TIMEOUT"]);

    QString val = unquoteWord(m_settings.value("GRUB_DEFAULT"));
    // qWarning() <<val << "GRUB_DEFAULT";
    if (val == "saved") {
        m_defaultEntryType_orig = GrubData::DefaultEntryType::PreviouslyBooted;
    } else {
        m_defaultEntry_orig = val;
        m_defaultEntryType_orig = GrubData::DefaultEntryType::Predefined;
    }

    if (m_settings.contains("GRUB_HIDDEN_TIMEOUT")) {
        m_hiddenTimeout_orig = unquoteWord(m_settings["GRUB_HIDDEN_TIMEOUT"]).toFloat();
    } else {
        m_hiddenTimeout_orig = 0;
    }
    if (m_settings.contains("GRUB_DISABLE_OS_PROBER")) {
        // TODO check what happens when not found
        m_lookForOtherOs_orig = !(unquoteWord(m_settings["GRUB_DISABLE_OS_PROBER"]) == "true");
    }

    m_timeout = m_timeout_orig;
    m_hiddenTimeout = m_hiddenTimeout_orig;
    m_defaultEntryType = m_defaultEntryType_orig;
    m_lookForOtherOs = m_lookForOtherOs_orig;
    m_defaultEntry = m_defaultEntry_orig;
    // qWarning() << m_defaultEntry<< m_defaultEntryType;
    Q_EMIT dataChanged();
}

void GrubData::set()
{
    // qWarning() << "set was called";
    // qWarning() << m_timeout;
    // KAuth::Action readAction("org.kde.kcontrol.kcmgrub2.testaction");
    // readAction.setHelperId("org.kde.kcontrol.kcmgrub2");
    // KAuth::ExecuteJob *job = readAction.execute();
    // if (!job->exec()) {
    //     qDebug() << "KAuth returned an error code:" << job->error();
    // } else {
    //     QString contents = job->data()["contents"].toString();
    // }
    // qWarning() << "default entry type" << m_defaultEntryType;
    Q_EMIT dataChanged();
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
            setValue("GRUB_DEFAULT", quoteWord(m_defaultEntry));
        }
    } else if (m_defaultEntryType_orig == DefaultEntryType::Predefined && m_defaultEntry != m_defaultEntry_orig) {
        // qWarning() << m_defaultEntry;
        setValue("GRUB_DEFAULT", quoteWord(m_defaultEntry));
    }

    KAuth::Action readAction("org.kde.kcontrol.kcmgrub2.save");
    readAction.setHelperId("org.kde.kcontrol.kcmgrub2");
    readAction.addArgument("homeDir", qgetenv("HOME"));
    KAuth::ExecuteJob *job = readAction.execute();
    if (!job->exec()) {
        qWarning() << "KAuth returned an error code:" << job->error();
    } else {
        QString contents = job->data()["result"].toString();
        qWarning() << "KAuth result:" << contents;
    }
    readAll();
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
    return (m_timeout != m_timeout_orig) || (m_hiddenTimeout != m_hiddenTimeout_orig) || (m_lookForOtherOs != m_lookForOtherOs_orig)
        || (m_defaultEntryType != m_defaultEntryType_orig) || (m_defaultEntry != m_defaultEntry_orig);
}

void GrubData::readAll(){
    QByteArray fileContents;
    LoadOperations operations = NoOperation;
    // qWarning() << "File contents are being read";

    if (readFile("/boot/grub/grub.cfg", fileContents)) {
        parseEntries(QString::fromUtf8(fileContents.constData()));
    } else {
        operations |= MenuFile;
    }
    if (readFile(m_currFileName, fileContents)) {
        parseSettings(QString::fromUtf8(fileContents.constData()));
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
}

QStringList GrubData::getAllOsEntries(){
    QStringList entries;
    for (const Entry &osEntry :qAsConst(m_osEntries)){
        if (osEntry.type() == Entry::Menuentry) {
            entries << osEntry.fullTitle();
        }
    }
    return entries;
}