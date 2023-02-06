#include "entry.h"
#include "grubdata.h"
#include "testutils.h"
#include <QtTest/QtTest>

class TestInvalidDefault : public QObject
{
    Q_OBJECT
private slots:
    // Check that 0 indexed entry is used as grub default when GRUB_DEFAULT is invalid
    void loadGrubDefault()
    {
        auto _data = new GrubData();
        const QString filePath = getTestFilesDir() + "/invalid_default.txt";
        _data->setCurrentFile(filePath);

        auto defaultProp = getDefaultEntryProp(_data);
        auto defaultEntry = defaultProp.read(_data).value<Entry *>();
        QVERIFY(defaultEntry->fullNumTitle() == "0");
    }
};

QTEST_MAIN(TestInvalidDefault)
#include "invaliddefault.moc"
