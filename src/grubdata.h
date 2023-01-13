#pragma once
#include <QHash>
#include <QObject>
#include <entry.h>

QStringList getAllOsEntries();

class GrubData : public QObject{

    Q_OBJECT
    Q_PROPERTY(float grubTimeout READ grubTimeout)
    // Q_PROPERTY(bool predefined READ predefined)
    // Q_PROPERTY(bool previouslyBooted READ previouslyBooted)
    // Q_PROPERTY(defaultEntryType defaultEntryType READ getDefaultEntryType)
    Q_PROPERTY(QStringList osEntries READ getAllOsEntries);
    Q_PROPERTY(bool showMenu READ showMenu)
    Q_PROPERTY(bool bootDefault READ bootDefault)
    // Q_PROPERTY(bool lookForOtherOs READ lookForOtherOs)
    



public:
    explicit GrubData(QObject *parent = nullptr);
    QStringList bootEntries;


    enum defaultEntryType{
            Predefined = 0,
            PreviouslyBooted = 1
    };
    Q_ENUM(defaultEntryType)

    // Q_INVOKABLE void set(QString cob_osEntries , bool chb_showMenu);

private:
    QString parseTitle(const QString &line);
    void parseEntries(const QString &line);
    bool readFile(const QString &filename, QByteArray &fileContents);
    void readAll();
    void parseSettings(const QString &config);
    void parseValues();

    // void load();
    // bool predefined();
    // bool previouslyBooted();

    // GrubData::defaultEntryType getDefaultEntryType();
    QStringList getAllOsEntries();
    bool showMenu();
    bool bootDefault();
    float grubTimeout();
    bool lookForOtherOs();

    QHash<QString, QString> m_settings;

    bool m_bootDefaultAfter;
    bool m_lookForOtherOs;
    float m_grubTimeout;
    bool m_showMenu;
    GrubData::defaultEntryType m_defaultEntryType;
    QString m_grubDefault;
    QList<Entry> m_osEntries;
    QStringList m_issues;
    QString m_currFileName;

};