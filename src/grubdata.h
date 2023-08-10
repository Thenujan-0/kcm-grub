#pragma once
#include <QDBusAbstractAdaptor>
#include <QHash>
#include <QObject>

#include <kscreen/getconfigoperation.h>

#include <entry.h>

QStringList getAllOsEntries();
class Language;

class Resolution
{
    Q_GADGET;
    Q_PROPERTY(QString name MEMBER name);
    Q_PROPERTY(QString value MEMBER value);

public:
    QString name;
    QString value;
};
Q_DECLARE_METATYPE(Resolution);

bool operator==(const Resolution &lhs, const Resolution &rhs);

class GrubData : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kcontrol.kcmgrub");
    Q_PROPERTY(QList<Entry *> osEntries MEMBER m_osEntries NOTIFY osEntriesChanged);
    Q_PROPERTY(Entry *defaultEntry MEMBER m_defaultEntry NOTIFY dataChanged);
    Q_PROPERTY(GrubData::DefaultEntryType defaultEntryType MEMBER m_defaultEntryType NOTIFY dataChanged);
    Q_PROPERTY(float timeout MEMBER m_timeout NOTIFY dataChanged);
    Q_PROPERTY(bool immediateTimeout MEMBER m_immediateTimeout NOTIFY dataChanged);
    Q_PROPERTY(QString timeoutStyle MEMBER m_timeoutStyle NOTIFY dataChanged);
    Q_PROPERTY(bool lookForOtherOs MEMBER m_lookForOtherOs NOTIFY dataChanged);
    Q_PROPERTY(QList<Language *> languages MEMBER m_languages NOTIFY dataChanged);
    Q_PROPERTY(Language *language MEMBER m_language NOTIFY dataChanged);
    Q_PROPERTY(bool generateRecoveryEntries MEMBER generateRecoveryEntries NOTIFY dataChanged);
    Q_PROPERTY(QList<Resolution> grubResolutions MEMBER m_grubResolutions NOTIFY resolutionsChanged);
    Q_PROPERTY(QList<Resolution> linuxKernelResolutions MEMBER m_linuxKernelResolutions NOTIFY resolutionsChanged);
    Q_PROPERTY(QString grubResolution MEMBER m_grubResolution NOTIFY dataChanged);
    Q_PROPERTY(QString linuxKernelResolution MEMBER m_linuxKernelResolution NOTIFY dataChanged);
    Q_PROPERTY(QString colorNormalForeground MEMBER m_colorNormalForeground NOTIFY dataChanged);
    Q_PROPERTY(QString colorNormalBackground MEMBER m_colorNormalBackground NOTIFY dataChanged);
    Q_PROPERTY(QString colorHighlightForeground MEMBER m_colorHighlightForeground NOTIFY dataChanged);
    Q_PROPERTY(QString colorHighlightBackground MEMBER m_colorHighlightBackground NOTIFY dataChanged);

public:
    explicit GrubData(QObject *parent = nullptr);
    QStringList bootEntries;

    enum DefaultEntryType { Predefined = 0, PreviouslyBooted = 1 };
    Q_ENUM(DefaultEntryType)

    bool isDirty();
    void save();
    void load();
    bool setValue(QString key, QString val, QString readFileName = "");
    bool unsetValue(QString key, QString readFileName = "");
    void initCache();
    void setCurrentFile(const QString &fileName);
    QString getValue(const QString &key);

    Q_INVOKABLE void changeMenuPermissions();

signals:
    void dataChanged();
    void resolutionsChanged();
    void osEntriesChanged();
    void error(const QString &message);
    void savingStarted();
    void updateOutput(QString text);
    void savingFinished();
    void menuNotReadable();

public Q_SLOTS:
    bool updateCommandOutput(QString text);
    void emitSavingStarted();

private:
    QString parseTitle(const QString &line);
    void parseEntries(const QString &line);
    void readAll();
    void parseSettings(const QString &config);
    void showLocales();
    void configReceived(KScreen::ConfigOperation *op);
    Language *findLanguage(const QString &locale);

    // Adds default values for keys that don't exist in /etc/default/grub
    void addDefaultValues();
    void parseValues();
    void parseEnv(const QString &config);

    void deleteEntries();
    Entry *findEntry(const QString &value);

    QHash<QString, QString> m_settings;

    bool m_lookForOtherOs;
    bool m_lookForOtherOs_orig;
    bool m_immediateTimeout; // If this is true then value of timeout is ignored and zero is used as the timeout value
    float m_timeout;
    float m_timeout_orig;
    QString m_timeoutStyle;
    QString m_timeoutStyle_orig;
    DefaultEntryType m_defaultEntryType;
    DefaultEntryType m_defaultEntryType_orig;
    Entry *m_defaultEntry;
    Entry *m_defaultEntry_orig;
    QList<Entry *> m_osEntries;
    QStringList m_issues;
    QString m_currFileName;
    Language *m_language;
    Language *m_language_orig;
    bool generateRecoveryEntries;
    bool generateRecoveryEntries_orig;

    QString m_grubResolution;
    QString m_grubResolution_orig;
    QString m_linuxKernelResolution;
    QString m_linuxKernelResolution_orig;
    QString m_colorNormalForeground;
    QString m_colorNormalForeground_orig;
    QString m_colorNormalBackground;
    QString m_colorNormalBackground_orig;
    QString m_colorHighlightForeground;
    QString m_colorHighlightForeground_orig;
    QString m_colorHighlightBackground;
    QString m_colorHighlightBackground_orig;

    QHash<QString, QString> m_env;
    bool m_memtest;
    bool m_memtestOn;
    QHash<QString, QString> m_devices;
    QStringList m_resolutions;
    bool m_resolutionsEmpty;
    bool m_resolutionsForceRead;
    QStringList m_locales;
    QList<Language *> m_languages;
    QList<Resolution> m_grubResolutions;
    QList<Resolution> m_linuxKernelResolutions;
};

class Language : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString formattedName READ formattedName CONSTANT);

public:
    Language(const QString &argName, const QString &argLocale);
    QString formattedName();
    QString name;
    QString locale;
};
bool operator==(const Language &left, const Language &right);
Q_DECLARE_TYPEINFO(Language, Q_MOVABLE_TYPE);
