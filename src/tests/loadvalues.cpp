#include "loadvalues.h"
#include <filesystem>
#include <grubdata.h>

#define compareValue(key, value) compareValue_(_data, key, value);

QString cwd = QString::fromStdString(std::filesystem::current_path().parent_path().string());
const QString PATH = "/home/thenujan/Desktop/Code/kcm-grub/";

void compareValue_(GrubData *data, QString key, QString val)
{
    QCOMPARE(data->getValue(key), val);
}

// Check if the default values for GRUB_DEFAULT,GRUB_TIMEOUT are used when keys are not set in config
void TestLoadValues::defaultValues()
{
    auto _data = new GrubData();
    const QString filePath = cwd + "/src/tests/testCaseFiles/commentedLineTest.txt";
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
    const QString filePath = cwd + "/src/tests/testCaseFiles/grub.txt";
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