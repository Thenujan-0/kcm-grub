#include <QtTest/QtTest>

class TestLoadValues : public QObject
{
    Q_OBJECT

private slots:
    void defaultValues();
    void loadsAllValuesCorrectly();
};