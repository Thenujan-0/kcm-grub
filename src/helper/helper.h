#ifndef HELPER_H
#define HELPER_H

// KDE
#include <KAuth/ActionReply>

using namespace KAuth;

class Helper : public QObject
{
    Q_OBJECT
public:
    Helper();

private:
    ActionReply executeCommand(const QStringList &command);
    // bool setLang(const QString &lang);
    // void updateGrub(QDBusConnection bus,ActionReply reply);
public Q_SLOTS:
    ActionReply save(QVariantMap args);

private:
    // QString errorDescription(int errorCode, const QString &errorMessage) const;
};

#endif
