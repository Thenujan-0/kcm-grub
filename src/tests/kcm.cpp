#include "kcm.h"
#include "grubdata.h"

Kcm::Kcm(QObject *parent)
    : m_data(new GrubData)
{
    Q_UNUSED(parent);
}

Kcm::~Kcm()
{
}

GrubData *Kcm::grubData() const
{
    return m_data;
};
