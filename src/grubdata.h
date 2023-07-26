#pragma once
#include <QDBusAbstractAdaptor>
#include <QHash>
#include <QObject>

#include <entry.h>

QStringList getAllOsEntries();

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

public:
    explicit GrubData(QObject *parent = nullptr);
    QStringList bootEntries;

    enum DefaultEntryType { Predefined = 0, PreviouslyBooted = 1 };
    Q_ENUM(DefaultEntryType)

    bool isDirty();
    void save();
    void load();
    bool setValue(QString key, QString val, QString readFileName = "");
    void initCache();
    void setCurrentFile(const QString &fileName);
    QString getValue(const QString &key);

signals:
    void dataChanged();
    void osEntriesChanged();
    void error(const QString &message);
    void savingStarted();
    void updateOutput(QString text);
    void savingFinished();

public Q_SLOTS:
    bool updateCommandOutput(QString text);
    void emitSavingStarted();

private:
    QString parseTitle(const QString &line);
    void parseEntries(const QString &line);
    void readAll();
    void parseSettings(const QString &config);
    void showLocales();

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

    QHash<QString, QString> m_env;
    bool m_memtest;
    bool m_memtestOn;
    QHash<QString, QString> m_devices;
    QStringList m_resolutions;
    bool m_resolutionsEmpty;
    bool m_resolutionsForceRead;
    QStringList m_locales;
};