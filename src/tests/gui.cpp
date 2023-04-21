#include "grubdata.h"
#include "kcm.h"
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTest>
#include <QUrl>
#include <QtTest>

class TestGui : public QObject

{
    Q_OBJECT
public:
    Kcm *m_kcm = new Kcm();
    TestGui()
    {
    }

public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
    }
    void cleanupTestCase()
    {
        // Clean up any resources used by the QML engine
        // (not strictly necessary in this example)
    }

private Q_SLOTS:
    void initTestCase()
    {
        QGuiApplication::setQuitOnLastWindowClosed(true);
        qmlRegisterAnonymousType<GrubData>("", 1);
        qmlRegisterAnonymousType<Entry>("", 1);
        qmlRegisterUncreatableType<GrubData>("org.kde.plasma.kcm.data", 1, 0, "DefaultEntry", QStringLiteral("Only for enums"));
        QString testFile = "/home/thenujan/Desktop/Code/kcm-grub/src/package/contents/ui/testWindow.qml";

        m_engine.rootContext()->setContextProperty("myContextProperty", QVariant(true));
        m_engine.rootContext()->setContextProperty("kcm", m_kcm);

        m_engine.load(testFile);
    }
    void test_disable_based_on_lookForOtherOs()
    {
        QObject *doubleSpinBox = findItem("dsb_timeout");
        QCOMPARE(doubleSpinBox->property("text").toString(), "10");

        QObject *chb_lookForOtherOs = findItem("chb_lookForOtherOs");
        auto rb_immediately = findItem("rb_immediately");
        auto opacity = rb_immediately->property("opacity");
        QVERIFY2(opacity.toDouble() < 1.0, (QString("opacity of immediately radio button is not less than 1.0 : ") + opacity.toString()).toUtf8().data());
        clickItem(chb_lookForOtherOs);
        opacity = rb_immediately->property("opacity");
        QVERIFY2(opacity.toDouble() == 1.0, (QString("opacity of immediately radio button is less than 1.0 : ") + opacity.toString()).toUtf8().data());

        // Todo verify the opacity of show menu as well
    }

    void clickItem(QObject *itemObject)
    {
        QQmlContext *rootContext = m_engine.rootContext();
        QWindow *pRootWindow;
        if (!rootContext) {
            qFatal("Root context is nullptr");
        }
        pRootWindow = qobject_cast<QQuickWindow *>(m_engine.rootObjects()[0]);

        QQuickItem *pItem = qobject_cast<QQuickItem *>(itemObject);
        if (!pRootWindow) {
            qFatal("Window not found");
        }
        auto oPointF = pItem->mapToScene(QPoint(0, 0));
        auto oPoint = oPointF.toPoint();
        oPoint.rx() += pItem->width() / 2;
        oPoint.ry() += pItem->height() / 2;
        QTest::mouseClick(pRootWindow, Qt::LeftButton, Qt::NoModifier, oPoint);
    }

    QObject *findItem(QString objectName)
    {
        QObject *root = m_engine.rootObjects()[0];
        QObject *item = root->findChild<QQuickItem *>(objectName);
        if (!item) {
            qFatal("%s", (objectName + QString(" couldn't be found. Stopping now using qFatal")).toUtf8().data());
        }
        return item;
    }

private:
    QQmlApplicationEngine m_engine;
};

QTEST_MAIN(TestGui)

#include "gui.moc"
