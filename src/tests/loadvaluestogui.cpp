#include "entry.h"
#include "grubdata.h"
#include "testutils.h"
#include <QtTest/QtTest>

class TestLoadValuesToGui : public QObject
{
    Q_OBJECT
private slots:
    // Check that 'no translation' is loaded correctly
    void loadTranslation()
    {
        auto _data = new GrubData();
        const QString filePath = getTestFilesDir() + "/grub.txt";
        _data->setCurrentFile(filePath);

        auto languageProp = getLanguageProp(_data);
        auto language = languageProp.read(_data).value<Language *>();
        QVERIFY(language->locale == "");
        QVERIFY(language->name == "No translation");
    }

    void loadGrubGfxMode()
    {
        auto _data = new GrubData();
        const QString filePath = getTestFilesDir() + "/grub.txt";
        _data->setCurrentFile(filePath);
    }
};

QTEST_MAIN(TestLoadValuesToGui)
#include "loadvaluestogui.moc"
