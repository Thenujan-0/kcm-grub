#pragma once
#include <QHash>
#include <QObject>
#include <entry.h>

QStringList getAllOsEntries();

class GrubData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<Entry *> osEntries MEMBER m_osEntries CONSTANT);
    Q_PROPERTY(Entry *defaultEntry MEMBER m_defaultEntry NOTIFY dataChanged);
    Q_PROPERTY(GrubData::DefaultEntryType defaultEntryType MEMBER m_defaultEntryType NOTIFY dataChanged);
    Q_PROPERTY(float hiddenTimeout MEMBER m_hiddenTimeout NOTIFY dataChanged);
    Q_PROPERTY(float timeout MEMBER m_timeout NOTIFY dataChanged);
    Q_PROPERTY(QString timeoutStyle MEMBER m_timeoutStyle NOTIFY dataChanged);
    Q_PROPERTY(bool lookForOtherOs MEMBER m_lookForOtherOs NOTIFY dataChanged);

public:
    explicit GrubData(QObject *parent = nullptr);
    QStringList bootEntries;

    enum DefaultEntryType { Predefined = 0, PreviouslyBooted = 1 };
    Q_ENUM(DefaultEntryType)

    bool isDirty();
    void save();
    bool setValue(QString key, QString val, QString readFileName = "");
    void initCache();
    void setCurrentFile(const QString &fileName);
    QString getValue(const QString &key);
signals:
    void dataChanged();

private:
    QString parseTitle(const QString &line);
    void parseEntries(const QString &line);
    void readAll();
    void parseSettings(const QString &config);

    // Adds default values for keys that don't exist in /etc/default/grub
    void addDefaultValues();
    void parseValues();

    void deleteEntries();
    Entry *findEntry(QString value);

    QHash<QString, QString> m_settings;

    bool m_lookForOtherOs;
    bool m_lookForOtherOs_orig;
    float m_timeout;
    float m_timeout_orig;
    float m_hiddenTimeout;
    float m_hiddenTimeout_orig;
    QString m_timeoutStyle;
    QString m_timeoutStyle_orig;
    DefaultEntryType m_defaultEntryType;
    DefaultEntryType m_defaultEntryType_orig;
    Entry *m_defaultEntry;
    Entry *m_defaultEntry_orig;
    QList<Entry *> m_osEntries;
    QStringList m_issues;
    QString m_currFileName;
};