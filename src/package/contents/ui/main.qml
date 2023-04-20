

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.0


import org.kde.kcm 1.4 as KCM
import org.kde.kirigami 2.14 as Kirigami
import org.kde.plasma.kcm.data 1.0

KCM.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.largeSpacing * 12 +cob_os_entries.implicitWidth
    implicitHeight: Kirigami.Units.largeSpacing * 70
    header: Kirigami.InlineMessage {
        id: errorMessage
        
        showCloseButton: true

        Connections {
            target: kcm.grubData
            function onError(message){
                errorMessage.type = Kirigami.MessageType.Error
                errorMessage.visible = true
                errorMessage.text = message
            }
        }

        property var fixItAction: Kirigami.Action {
                property string fileName
                text: i18n("Learn more")
                icon.name: "help-hint"
                onTriggered: {
                    errorMessage.visible = false
                    sheetOverrideWarning.open()
                }
        }
    }

    Kirigami.OverlaySheet {
        id: sheetSavingProgress
        property bool saving :false
        property int spacing :Kirigami.Units.largeSpacing

        contentItem:Item{

            implicitHeight: overlayMainlabel.implicitHeight + overlayProgressBar.implicitHeight + overlayButton.implicitHeight + sheetScrollView.implicitHeight + sheetSavingProgress.spacing * 3
            implicitWidth: sheetLabel.visible && (sheetLabel.implicitWidth > 300) ? sheetLabel.implicitWidth : 300

            QQC2.Label {
                id:overlayMainlabel
                Layout.fillWidth: true
                anchors.top:parent.top
                text: sheetSavingProgress.saving ?"Saving changes please wait!" : "Settings have been saved successfully"
            }
            QQC2.ProgressBar {
                id:overlayProgressBar
                visible: sheetSavingProgress.saving
                anchors.top:overlayMainlabel.bottom
                anchors.topMargin:sheetSavingProgress.spacing
                anchors.left: parent.left
                anchors.right: parent.right
                indeterminate: true
            }

            QQC2.Button{
                id:overlayButton
                text: "Show more details"
                anchors.top:overlayProgressBar.bottom
                anchors.topMargin:sheetSavingProgress.spacing
                anchors.left: parent.left
                anchors.right: parent.right

                onClicked: {
                    if (sheetScrollView.visible){
                        sheetScrollView.visible = false

                        text="Show more details"
                    }
                    else{
                        sheetScrollView.visible = true
                        text= "Hide more details"
                    }
                }
            }
            QQC2.ScrollView{
                id:sheetScrollView
                visible: false
                implicitHeight: visible? 150 : 0
                anchors.top :overlayButton.bottom
                anchors.topMargin:sheetSavingProgress.spacing
                anchors.bottom:parent.bottom
                Layout.fillWidth: true
                clip:true


                background:Rectangle{
                    id: rect
                    color:Kirigami.Theme.alternateBackgroundColor
                }

                QQC2.Label{
                    id:sheetLabel
                    
                    Connections {
                        target: kcm.grubData
                        function onSavingStarted(){
                            sheetSavingProgress.open()
                            sheetLabel.text = ""
                            sheetSavingProgress.saving = true
                        }

                        function onSavingFinished(){
                            sheetSavingProgress.saving = false
                        }
                    }

                    Connections {
                        target: kcm.grubData
                        function onUpdateOutput(text){
                            sheetLabel.text += text
                        }
                    }
                }
            }
        }
    }

    FormLayout{
        
    }
}
