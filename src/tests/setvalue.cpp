#include <KAuth/Action>

#include <QMetaProperty>

#include "common.h"
#include "grubdata.h"
#include "setvalue.h"
#include "testutils.h"

#include <optional>
#include <unistd.h>

using std::optional;

void TestSetValue::simple()
{
    auto data = new GrubData();
    data->initCache();
    const QString filePath = getDataPath() + "/grubToWrite.txt";

    // Might be false if the order of the registered Q_PROPERTY(s) changes
    QMetaProperty defaultEntryTypeProp = data->metaObject()->property(2);
    QMetaProperty defaultEntryProp = data->metaObject()->property(3);
    QString defaultEntry = defaultEntryProp.read(data).toString();
    QString defaultEntryType = defaultEntryTypeProp.read(data).toString();

    // Check if it read the values correctly
    qWarning() << defaultEntryType << defaultEntry;
    if (defaultEntryType == GrubData::DefaultEntryType::PreviouslyBooted) {
        QCOMPARE(getValue("GRUB_DEFAULT", filePath), "saved");
        QCOMPARE(getValue("GRUB_SAVED_DEFAULT", filePath), "true");
    }

    data->setValue("GRUB_DEFAULT", "FAKE default", filePath);

    // data->setCurrentFile(filePath);

    const QString newVal = getValue("GRUB_DEFAULT", filePath);
    QCOMPARE(newVal, "FAKE default");
}

// Check if commented key is replaced when that key is set
void TestSetValue::replaceCommented()
{
    auto data = new GrubData();
    QString testFile = getTestFilesDir() + "/commentedLineTest.txt";
    QString writeFile = getTestFilesDir() + "/temp.txt";
    copyTo(testFile, writeFile);

    optional<QString> state1 = readFile(writeFile);
    QVERIFY(state1);
    data->setCurrentFile(writeFile);

    QMetaProperty defaultEntryProp = data->metaObject()->property(2);
    QMetaProperty defaultEntryTypeProp = data->metaObject()->property(3);
    qWarning() << defaultEntryProp.read(data) << "default entry";

    QVERIFY(defaultEntryProp.write(data, "Manjaro linux is awesome"));
    data->save();
    optional<QString> state2 = readFile(writeFile);
    QVERIFY(state2);

    auto change = lineChanged(state1.value(), state2.value());
    QVERIFY(change);
    QCOMPARE(change.value()[0], "#GRUB_DEFAULT='Manjaro Linux'");
    QCOMPARE(change.value()[1], "GRUB_DEFAULT='Manjaro linux is awesome'");
    // QString defaultEntryType = defaultEntryTypeProp.read(data).toString();
}

// Check if key is correctly added when a key doesn't exist in the config
void TestSetValue::addKey()
{
    auto data = new GrubData();
    QString testFile = getTestFilesDir() + "/commentedLineTest.txt";
    QString writeFile = getTestFilesDir() + "/temp.txt";
    copyTo(testFile, writeFile);
    optional<QString> state1 = readFile(writeFile);
    QVERIFY(state1);
    data->setCurrentFile(writeFile);

    QMetaProperty lookForOtherOsProp = data->metaObject()->property(6);
    QVERIFY(lookForOtherOsProp.write(data, true));
    data->save();

    auto state2 = readFile(writeFile);
    QVERIFY(state2);
    auto addedLine = lineAdded(state1.value(), state2.value());
    QVERIFY(addedLine);
    QCOMPARE(addedLine.value(), "GRUB_DISABLE_OS_PROBER=false");
}

QTEST_MAIN(TestSetValue)
#include "setvalue.moc"