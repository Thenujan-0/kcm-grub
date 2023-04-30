#ifndef KCM_H
#define KCM_H

#include "QObject"
#include "grubdata.h"

class Kcm : public QObject
{
    Q_OBJECT
    Q_PROPERTY(GrubData *grubData READ grubData CONSTANT);

public:
    explicit Kcm(QObject *parent = nullptr);
    ~Kcm() override;
    GrubData *grubData() const;
    Q_INVOKABLE void settingsChanged();

private:
    GrubData *m_data;
    bool m_apply_btn_enabled;
};

#endif