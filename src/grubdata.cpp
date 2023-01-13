
#include <QDebug>
#include <QFile>

#include <entry.h>
#include <grubdata.h>
#include <common.h>


GrubData::GrubData(QObject *parent)
    :m_issues(QStringList()),
    m_currFileName(QString("/etc/default/grub"))
{
//      showMenu = true;
    readAll();
    qWarning()<<"loaded everything here";
    for (const Entry &osEntry :qAsConst(m_osEntries)){
        qWarning() <<osEntry.fullTitle();
    }
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
    QString line, configStr = config;
    QTextStream stream(&configStr, QIODevice::ReadOnly | QIODevice::Text);

    m_settings.clear();
    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();
        if (line.contains(QRegExp(QLatin1String("^(GRUB_|LANGUAGE=)")))) {
            m_settings[line.section(QLatin1Char('='), 0, 0)] = line.section(QLatin1Char('='), 1);
        }
    }
    parseValues();
}
void GrubData::parseValues()
{
    if (m_settings["GRUB_TIMEOUT_STYLE"] == "hidden") {
        m_showMenu = true;
    } else {
        m_showMenu = false;
    }

    if (m_settings.contains("GRUB_TIMEOUT")) {
        m_grubTimeout = unquoteWord(m_settings["GRUB_TIMEOUT"]).toFloat();
        qWarning() << m_grubTimeout << "timeout" << unquoteWord(m_settings["GRUB_TIMEOUT"]);
    }
}

bool GrubData::showMenu()
{
    return m_showMenu;
}

bool GrubData::bootDefault()
{
    return m_bootDefaultAfter;
}

float GrubData::grubTimeout()
{
    qWarning() << "returned";
    return m_grubTimeout;
}
bool GrubData::lookForOtherOs()
{
    return m_lookForOtherOs;
}

void GrubData::readAll(){
    QByteArray fileContents;
    LoadOperations operations = NoOperation;
    qWarning() << "File contents are being read";

    if (readFile("/boot/grub/grub.cfg", fileContents)) {
        parseEntries(QString::fromUtf8(fileContents.constData()));
    } else {
        operations |= MenuFile;
    }
    if (readFile("/etc/default/grub", fileContents)) {
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
        entries <<osEntry.fullTitle();
    }
    return entries;
}