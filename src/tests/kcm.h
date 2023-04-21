#ifndef KCM_H
#define KCM_H

#include "QObject"
#include "grubdata.h"

class Kcm : public QObject
{
    Q_OBJECT
    Q_PROPERTY(GrubData *grubData READ grubData CONSTANT);
    // Todo add QInvokable settingsChanged

public:
    explicit Kcm(QObject *parent = nullptr);
    ~Kcm() override;
    GrubData *grubData() const;

private:
    GrubData *m_data;
};

#endif