import QtQuick 2.0
import QtQuick.Controls 2.1 as QQC2

Item{
    id:customTextField
    width:textField.width
    height:textField.height

    property alias text : textField.text
    property var onDecrease;
    property var onIncrease;
    signal editFinished()

    QQC2.TextField{
        z:0
        id:textField
        anchors.top:customTextField.top 
        anchors.left:customTextField.left
        anchors.right:customTextField.right

        Component.onCompleted: textField.onEditingFinished.connect(editFinished)
        
    }
    QQC2.ToolButton{
        id:addButton
        z:1
        anchors.right:reduceButton.left
        icon.name: "add"
        onClicked: onIncrease()
    }
    QQC2.ToolButton{
        id:reduceButton
        z:1
        anchors.right:textField.right
        icon.name: "remove"
        onClicked: onDecrease()
        
        Component.onCompleted: {
            reduceButton.clicked.connect(textField.editingFinished)
        }
        
    }
}