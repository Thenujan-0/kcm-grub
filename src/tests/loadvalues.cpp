#include "loadvalues.h"

#include "testutils.h"
#include <grubdata.h>

#include <filesystem>

#define compareValue(key, value) compareValue_(_data, key, value);

void compareValue_(GrubData *data, QString key, QString val)
{
    QCOMPARE(data->getValue(key), val);
}

// Check if the default values for GRUB_DEFAULT,GRUB_TIMEOUT are used when keys are not set in config
void TestLoadValues::defaultValues()
{
    auto _data = new GrubData();
    const QString filePath = getTestFilesDir() + "/commentedLineTest.txt";
    QVERIFY(QFile::exists(filePath));
    _data->setCurrentFile(filePath);
    compareValue("GRUB_DEFAULT", "0");
    compareValue("GRUB_TIMEOUT", "5");
    compareValue("GRUB_DISABLE_OS_PROBER", "true");
    compareValue("GRUB_TIMEOUT_STYLE", "menu");
}

void TestLoadValues::loadsAllValuesCorrectly()
{
    auto _data = new GrubData();
    const QString filePath = getTestFilesDir() + "/grub.txt";
    QVERIFY(QFile::exists(filePath));
    _data->setCurrentFile(filePath);
    compareValue("GRUB_TIMEOUT", "\"11\"");
    compareValue("GRUB_DEFAULT", "saved");
    compareValue("GRUB_TIMEOUT_STYLE", "hidden");
    compareValue("GRUB_HIDDEN_TIMEOUT", "4");
    compareValue("GRUB_DISABLE_OS_PROBER", "false");
}

QTEST_MAIN(TestLoadValues)
#include "loadvalues.moc"