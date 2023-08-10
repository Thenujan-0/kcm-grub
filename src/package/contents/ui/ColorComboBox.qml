import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.14 as Kirigami

 QQC2.ComboBox{
    textRole: "text"
    delegate: QQC2.ItemDelegate {
        id: itemDelegate
        text: modelData.text
        width: parent.width
        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Rectangle {
                height: parent.height - Kirigami.Units.smallSpacing
                width: height
                color: modelData.color
                Layout.alignment: Qt.AlignLeft
            }
            Text {
                id:textElem
                rightPadding: itemDelegate.spacing
                text: itemDelegate.text
                Layout.alignment: Qt.AlignLeft
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }
}