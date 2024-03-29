
#include <QCollator>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QTimer>
#include <QtGlobal>

#include <common.h>
#include <config.h>
#include <entry.h>
#include <grubdata.h>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KIO/CopyJob>
#include <KJob>
#include <KLocalizedString>
#include <unistd.h>

#include "kscreen/output.h"
#include <kscreen/config.h>
#include <kscreen/configmonitor.h>
#include <kscreen/mode.h>

QString HOME = qgetenv("HOME");

static const QString TEMPDATAFILE = HOME + QString("/.local/share/grub-editor-cpp/grubToWrite.txt");

bool operator==(const Resolution &lhs, const Resolution &rhs)
{
    return lhs.value == rhs.value;
}

GrubData::GrubData(QObject *parent)
    : m_issues(QStringList())
    , m_currFileName(grubConfigPath())
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

void GrubData::showLocales()
{
    m_languages << new Language("No translation", "");
    for (const QString &locale : qAsConst(m_locales)) {
        QString language = QLocale(locale).nativeLanguageName();
        if (language.isEmpty()) {
            language = QLocale(locale.split(QLatin1Char('@')).first()).nativeLanguageName();
            if (language.isEmpty()) {
                language = QLocale(locale.split(QLatin1Char('@')).first().split(QLatin1Char('_')).first()).nativeLanguageName();
            }
        }
        m_languages << new Language(language, locale);
    }
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
}

void GrubData::addDefaultValues()
{
    QMap<QString, QString> map;
    map["GRUB_TIMEOUT"] = "5";
    map["GRUB_DEFAULT"] = "0";
    map["GRUB_DISABLE_OS_PROBER"] = "true";
    map["GRUB_TIMEOUT_STYLE"] = "menu";
    map["GRUB_DISABLE_RECOVERY"] = "false";
    map["GRUB_COLOR_NORMAL"] = "light-gray/black";
    map["GRUB_COLOR_HIGHLIGHT"] = "black/light-gray";

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

Language *GrubData::findLanguage(const QString &locale)
{
    QList<Language *>::iterator i;
    for (i = m_languages.begin(); i != m_languages.end(); ++i) {
        Language *language = *i;
        if (language->locale == locale) {
            return language;
        }
    }
    qWarning() << "Couldn't find locale, invalid locale " << locale;
    Language *language = new Language(locale, locale);
    m_languages << language;
    return language;
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

    generateRecoveryEntries_orig = m_settings.value("GRUB_DISABLE_RECOVERY") == "true";
    m_grubResolution_orig = unquoteWord(m_settings.value(QStringLiteral("GRUB_GFXMODE")));
    if (m_grubResolution_orig.isEmpty()) {
        m_grubResolution_orig = QStringLiteral("auto");
    }
    m_grubResolutions.clear();

    if (m_grubResolution_orig != QLatin1String("auto") && !m_resolutions.contains(m_grubResolution_orig)) {
        m_resolutions.append(m_grubResolution_orig);
    }
    m_linuxKernelResolution_orig = unquoteWord(m_settings.value(QStringLiteral("GRUB_GFXPAYLOAD_LINUX")));
    if (!m_linuxKernelResolution.isEmpty() && m_linuxKernelResolution != QLatin1String("text") && m_linuxKernelResolution != QLatin1String("keep")
        && !m_resolutions.contains(m_linuxKernelResolution)) {
        m_resolutions.append(m_linuxKernelResolution);
    }

    m_lookForOtherOs_orig = !(unquoteWord(m_settings["GRUB_DISABLE_OS_PROBER"]) == "true");
    m_language_orig = findLanguage(unquoteWord(m_settings.value("LANGUAGE")));

    m_colorNormalForeground_orig = unquoteWord(m_settings.value("GRUB_COLOR_NORMAL")).split(QLatin1Char('/'))[0];
    m_colorNormalBackground_orig = unquoteWord(m_settings.value("GRUB_COLOR_NORMAL")).split(QLatin1Char('/'))[1];

    m_colorHighlightForeground_orig = unquoteWord(m_settings.value("GRUB_COLOR_HIGHLIGHT")).split(QLatin1Char('/'))[0];
    m_colorHighlightBackground_orig = unquoteWord(m_settings.value("GRUB_COLOR_HIGHLIGHT")).split(QLatin1Char('/'))[1];

    m_timeout = m_timeout_orig;
    m_immediateTimeout = m_timeout == 0;
    m_timeoutStyle = m_timeoutStyle_orig;
    m_defaultEntryType = m_defaultEntryType_orig;
    m_lookForOtherOs = m_lookForOtherOs_orig;
    m_defaultEntry = m_defaultEntry_orig;
    m_language = m_language_orig;
    generateRecoveryEntries = generateRecoveryEntries_orig;
    m_grubResolution = m_grubResolution_orig;
    m_linuxKernelResolution = m_linuxKernelResolution_orig;
    m_colorNormalForeground = m_colorNormalForeground_orig;
    m_colorHighlightForeground = m_colorHighlightForeground_orig;
    m_colorNormalBackground = m_colorNormalBackground_orig;
    m_colorHighlightBackground = m_colorHighlightBackground_orig;
    m_grubResolutions.clear();
    m_linuxKernelResolutions.clear();

    KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation();
    connect(op, &KScreen::GetConfigOperation::finished, this, [this](KScreen::ConfigOperation *op) {
        configReceived(op);
    });
}

void GrubData::configReceived(KScreen::ConfigOperation *op)
{
    m_grubResolutions.clear();
    m_linuxKernelResolutions.clear();
    m_grubResolutions << Resolution{"Custom...", "custom"} << Resolution{"Auto", "auto"};
    m_linuxKernelResolutions << Resolution{"Custom...", "custom"} << Resolution{"Auto", "auto"} << Resolution{"Unspecified", ""}
                             << Resolution{"Boot in text mode", "text"} << Resolution{"Keep GRUB's resolution", "keep"};

    auto config = op->config();
    for (const auto &output : config->outputs()) {
        const auto modes = output->modes();
        auto modeKeys = modes.keys();
        QCollator collator;
        collator.setNumericMode(true);
        std::sort(modeKeys.begin(), modeKeys.end(), collator);
        for (const auto &key : modeKeys) {
            auto mode = *modes.find(key);

            auto name = QStringLiteral("%1x%2").arg(QString::number(mode->size().width()), QString::number(mode->size().height()));
            m_resolutions << name;
        }
    }

    for (const QString &resolution : m_resolutions) {
        m_grubResolutions << Resolution{resolution, resolution};
        m_linuxKernelResolutions << Resolution{resolution, resolution};
        Q_EMIT resolutionsChanged();
    }
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
        setValue("GRUB_DEFAULT", quoteWord(m_defaultEntry->fullNumTitle()));
    }

    float timeout = m_timeout;
    if (m_immediateTimeout) {
        timeout = 0.0;
    }

    if (timeout != m_timeout_orig) {
        setValue("GRUB_TIMEOUT", quoteWord(QString::number(timeout)));
    }

    if (m_lookForOtherOs != m_lookForOtherOs_orig) {
        const QString value = m_lookForOtherOs ? "false" : "true";
        setValue("GRUB_DISABLE_OS_PROBER", value);
    }
    if (m_timeoutStyle != m_timeoutStyle_orig) {
        setValue("GRUB_TIMEOUT_STYLE", m_timeoutStyle);
    }
    if (m_language != m_language_orig) {
        setValue("LANGUAGE", m_language->locale);
    }
    if (m_grubResolution != m_grubResolution_orig) {
        setValue("GRUB_GFXMODE", m_grubResolution);
    }
    if (m_linuxKernelResolution != m_linuxKernelResolution_orig) {
        setValue("GRUB_GFXPAYLOAD_LINUX", m_linuxKernelResolution);
    }
    if (m_colorHighlightBackground != m_colorHighlightBackground_orig || m_colorHighlightForeground != m_colorHighlightForeground_orig) {
        setValue("GRUB_COLOR_HIGHLIGHT", quoteWord(m_colorHighlightForeground + QLatin1Char('/') + m_colorHighlightBackground));
    }
    if (m_colorNormalBackground != m_colorNormalBackground_orig || m_colorNormalForeground != m_colorNormalForeground_orig) {
        setValue("GRUB_COLOR_NORMAL", quoteWord(m_colorNormalForeground + QLatin1Char('/') + m_colorNormalBackground));
    }

    QString sourceFilePath = qgetenv("HOME") + "/.local/share/grub-editor-cpp" + "/grubToWrite.txt";
    bool rootCopyNeeded = true;
    QFile file(m_currFileName);
    if (file.permissions() & QFile::WriteUser) {
        bool ok = copyTo(sourceFilePath, m_currFileName);
        rootCopyNeeded = !ok;
    }

    KAuth::Action saveAction("org.kde.kcontrol.kcmgrub.save");
    saveAction.setHelperId("org.kde.kcontrol.kcmgrub");
    saveAction.addArgument("sourceFilePath", sourceFilePath);
    saveAction.addArgument("saveFile", m_currFileName);
    saveAction.addArgument("rootCopyNeeded", rootCopyNeeded);
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
    QFile file(readFileName);
    // qWarning() << readFileName;
    QString toWrite;
    bool replaced = false;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);

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
    file.close();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    out << toWrite;
    file.close();
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
    float timeout = m_timeout;
    if (m_immediateTimeout) {
        timeout = 0.0;
    }
    // clang-format off
    return (timeout != m_timeout_orig) 
        || (m_lookForOtherOs != m_lookForOtherOs_orig) 
        || (m_defaultEntryType != m_defaultEntryType_orig)
        || (m_defaultEntry->fullTitle() != m_defaultEntry_orig->fullTitle()) 
        || (m_language != m_language_orig)
        || (m_grubResolution != m_grubResolution_orig)
        || (m_linuxKernelResolution != m_linuxKernelResolution_orig)
        || (m_colorHighlightBackground != m_colorHighlightBackground_orig)
        || (m_colorHighlightForeground != m_colorHighlightForeground_orig)
        || (m_colorNormalBackground != m_colorNormalBackground_orig)
        || (m_colorNormalForeground != m_colorNormalForeground_orig);
    // clang-format on
}

void GrubData::readAll(){
    optional<QString> fileContents;
    LoadOperations operations = NoOperation;

    fileContents = readFile(grubMenuPath());
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
    fileContents = readFile(grubEnvPath());
    if (fileContents.has_value()) {
        parseEnv(fileContents.value());
    } else if (QFile(grubEnvPath()).exists()) {
        operations |= EnvironmentFile;
    }
    if (QFile::exists(grubMemtestPath())) {
        m_memtest = true;
        m_memtestOn = (bool)(QFile::permissions(grubMemtestPath()) & (QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther));
    } else {
        operations |= MemtestFile;
    }

    if (QFileInfo(grubLocalePath()).isReadable()) {
        m_locales = QDir(grubLocalePath())
                        .entryList(QStringList() << QStringLiteral("*.mo"), QDir::Files)
                        .replaceInStrings(QRegExp(QLatin1String("\\.mo$")), QString());
    } else {
        operations |= Locales;
    }
    if (operations) {
        KAuth::Action loadAction(QStringLiteral("org.kde.kcontrol.kcmgrub.load"));
        loadAction.setHelperId(QStringLiteral("org.kde.kcontrol.kcmgrub"));
        loadAction.addArgument(QStringLiteral("operations"), (int)(operations));

        KAuth::ExecuteJob *loadJob = loadAction.execute();
        if (!loadJob->exec()) {
            qCritical() << "KAuth error!";
            qCritical() << "Error code:" << loadJob->error();
            qCritical() << "Error description:" << loadJob->errorText();
            Q_EMIT error(i18n("An error occured when loading configs"));
            return;
        }

        if (operations.testFlag(MenuFile)) {
            if (loadJob->data().value(QStringLiteral("menuSuccess")).toBool()) {
                if ((int)(operations) == MenuFile) {
                    Q_EMIT menuNotReadable();
                }
                parseEntries(QString::fromUtf8(loadJob->data().value(QStringLiteral("menuContents")).toByteArray().constData()));
            } else {
                qCritical() << "Helper failed to read file:" << grubMenuPath();
                qCritical() << "Error code:" << loadJob->data().value(QStringLiteral("menuError")).toInt();
                qCritical() << "Error description:" << loadJob->data().value(QStringLiteral("menuErrorString")).toString();
                Q_EMIT error(i18n("An error occured when loading configs"));
            }
        }
        if (operations.testFlag(ConfigurationFile)) {
            if (loadJob->data().value(QStringLiteral("configSuccess")).toBool()) {
                parseSettings(QString::fromUtf8(loadJob->data().value(QStringLiteral("configContents")).toByteArray().constData()));
            } else {
                qCritical() << "Helper failed to read file:" << grubConfigPath();
                qCritical() << "Error code:" << loadJob->data().value(QStringLiteral("configError")).toInt();
                qCritical() << "Error description:" << loadJob->data().value(QStringLiteral("configErrorString")).toString();
                Q_EMIT error(i18n("An error occured when loading configs"));
            }
        }
        if (operations.testFlag(EnvironmentFile)) {
            if (loadJob->data().value(QStringLiteral("envSuccess")).toBool()) {
                parseEnv(QString::fromUtf8(loadJob->data().value(QStringLiteral("envContents")).toByteArray().constData()));
            } else {
                qCritical() << "Helper failed to read file:" << grubEnvPath();
                qCritical() << "Error code:" << loadJob->data().value(QStringLiteral("envError")).toInt();
                qCritical() << "Error description:" << loadJob->data().value(QStringLiteral("envErrorString")).toString();
                Q_EMIT error(i18n("An error occured when loading configs"));
            }
        }
        if (operations.testFlag(MemtestFile)) {
            m_memtest = loadJob->data().value(QStringLiteral("memtest")).toBool();
            if (m_memtest) {
                m_memtestOn = loadJob->data().value(QStringLiteral("memtestOn")).toBool();
            }
        }
        if (operations.testFlag(Vbe)) {
            m_resolutions = loadJob->data().value(QStringLiteral("gfxmodes")).toStringList();
            m_resolutionsEmpty = false;
            m_resolutionsForceRead = false;
        }
        if (operations.testFlag(Locales)) {
            m_locales = loadJob->data().value(QStringLiteral("locales")).toStringList();
        }
    }
    showLocales();
    parseValues();
    Q_EMIT osEntriesChanged();
    Q_EMIT dataChanged();
}

void GrubData::changeMenuPermissions()
{
    KAuth::Action loadAction(QStringLiteral("org.kde.kcontrol.kcmgrub.changemenupermissions"));
    loadAction.setHelperId(QStringLiteral("org.kde.kcontrol.kcmgrub"));
    loadAction.addArgument(QStringLiteral("filePath"), grubMenuPath());

    KAuth::ExecuteJob *loadJob = loadAction.execute();
    if (!loadJob->exec()) {
        qCritical() << "KAuth error!";
        qCritical() << "Error code:" << loadJob->error();
        qCritical() << "Error description:" << loadJob->errorText();
        Q_EMIT error(i18n("An error occured when changing permissions"));
        return;
    }
}

void GrubData::parseEnv(const QString &config)
{
    QString line, configStr = config;
    QTextStream stream(&configStr, QIODevice::ReadOnly | QIODevice::Text);

    m_env.clear();
    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();
        if (line.startsWith(QLatin1Char('#'))) {
            continue;
        }
        m_env[line.section(QLatin1Char('='), 0, 0)] = line.section(QLatin1Char('='), 1);
    }
}

Language::Language(const QString &argName, const QString &argLocale)
{
    name = argName;
    locale = argLocale;
}

QString Language::formattedName()
{
    if (locale.isEmpty()) {
        return name;
    }
    return name + QLatin1Char('(') + locale + QLatin1Char(')');
}

bool operator==(const Language &left, const Language &right)
{
    return left.locale == right.locale;
}
