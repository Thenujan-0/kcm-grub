#include "setvalue.h"
#include <KAuth/Action>
#include <grubdata.h>
#include <unistd.h>
const QString PATH = qgetenv("HOME") + "/.local/share/grub-editor-cpp";

void TestSetValue::simple()
{
    auto data = new GrubData();
    data->initCache();
    const QString filePath = PATH + "/grubToWrite.txt";
    data->setValue("GRUB_DEFAULT", "FAKE default", filePath);
    data->setCurrentFile(filePath);

    const QString newVal = data->getValue("GRUB_DEFAULT");
    QCOMPARE(newVal, "FAKE default");
}

QTEST_MAIN(TestSetValue)
#include "setvalue.moc"