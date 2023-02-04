

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.0


import org.kde.kcm 1.4 as KCM
import org.kde.kirigami 2.14 as Kirigami
import org.kde.plasma.kcm.data 1.0
// import org.kde.kcms.grub2 1.0

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

    Kirigami.FormLayout{
        RowLayout{
            Kirigami.FormData.label:"Default entry:"

            QQC2.RadioButton{
                id:rb_pre_defined
                // checked:kcm.grubData.defaultEntryType == DefaultEntry.Predefined
                Binding on checked { value: kcm.grubData.defaultEntryType == DefaultEntry.Predefined }
                text:"predefined"

                onClicked: {
                    if(checked){
                        rb_previously_booted.checked=false
                    }
                    kcm.grubData.defaultEntryType = checked ? DefaultEntry.Predefined : DefaultEntry.PreviouslyBooted
                    kcm.grubData.defaultEntry = kcm.grubData.osEntries[cob_os_entries.currentIndex];
                    kcm.settingsChanged()
                }
            }
            QQC2.ComboBox{
                id:cob_os_entries
                model:kcm.grubData.osEntries
                textRole: "title"
                currentIndex:kcm.grubData.osEntries.indexOf(kcm.grubData.defaultEntry)

                onActivated: {
                    if (rb_pre_defined.checked){
                        kcm.grubData.defaultEntry = kcm.grubData.osEntries[cob_os_entries.currentIndex]
                    }
                    kcm.settingsChanged()
                }
            }
        }

        QQC2.RadioButton{
            id:rb_previously_booted
            text:"previously booted entry"
            // checked:kcm.grubData.defaultEntryType == DefaultEntry.PreviouslyBooted
            Binding on checked { value: kcm.grubData.defaultEntryType == DefaultEntry.PreviouslyBooted }
            onClicked: {
                if(checked){
                    rb_pre_defined.checked=false
                }
                kcm.grubData.defaultEntryType = checked ? DefaultEntry.PreviouslyBooted : DefaultEntry.Predefined
                kcm.settingsChanged()
            }
        }

        Item{
            Layout.preferredHeight:Kirigami.Units.largeSpacing
        }

        QQC2.CheckBox{
            Kirigami.FormData.label: "Menu:"
            id:automaticallyBootDefault
            text: "Automatically boot default entry"
            checked:kcm.grubData.timeout != -1
            onClicked: {
                if(!checked){
                    kcm.grubData.timeout = -1
                    kcm.settingsChanged()
                }
            }
        }

        RowLayout{
            QQC2.RadioButton{
                id:rb_immediately
                Layout.leftMargin: Kirigami.Units.largeSpacing * 3

                text:"Immediately"
                // Workaround for https://bugreports.qt.io/browse/QTBUG-30801

                opacity: automaticallyBootDefault.checked && !chb_lookForOtherOs.checked ? 1 : 0.5
                checked:kcm.grubData.timeout == 0
                onClicked: {
                    if (!(automaticallyBootDefault.checked && !chb_lookForOtherOs.checked)){
                        checked = false
                        return
                    }
                    if (checked) {
                        rb_after.checked = false;
                        kcm.grubData.timeout = 0;
                        kcm.settingsChanged();
                    }
                }

                Binding on checked {
                    value: kcm.grubData.timeout == 0
                }
                QQC2.ToolTip {
                    text: "Can't be enabled when \"Look for other operating systems\" is enabled"
                }
            }
        }
        RowLayout{
            QQC2.RadioButton{
                id:rb_after

                Layout.leftMargin: Kirigami.Units.largeSpacing * 3
                text:"After"
                enabled:automaticallyBootDefault.checked
                checked:kcm.grubData.timeout != 0
                onClicked: {
                    if(checked){
                        rb_immediately.checked=false
                        kcm.grubData.timeout = Number(timeout.text) > 0 ? Number(timeout.text) : 10
                        kcm.settingsChanged()
                    }
                }

                Binding on checked {
                    value: kcm.grubData.timeout != 0
                }

            }

            DoubleSpinBox {
                id: timeout

                text: kcm.grubData.timeout.toString()
                onIncrease: function(){
                    kcm.grubData.timeout += 1
                    kcm.settingsChanged()
                }
                onDecrease: function(){
                    if (kcm.grubData.timeout <= 1)
                        return ;

                    kcm.grubData.timeout -= 1;
                    kcm.settingsChanged();
                }

                onEditFinished: {
                    if(rb_after.checked){
                        kcm.grubData.timeout = Number(text)
                        kcm.settingsChanged()
                    }
                }
            }

            QQC2.Label{
                text:"seconds"
            }
        }
        QQC2.CheckBox {
            id: chb_showMenu
            text: "Show menu"
            opacity: !chb_lookForOtherOs.checked ? 1 :0.5
            
            onClicked: {
                if (chb_lookForOtherOs.checked){
                    checked = true
                    return
                }
                if (checked)
                    kcm.grubData.timeoutStyle = "menu";
                else
                    kcm.grubData.timeoutStyle = "hidden";
            }

            Binding on checked {
                value: kcm.grubData.timeoutStyle == "menu"
            }
            QQC2.ToolTip{
                text:"Can't be disabled when look for other os is enabled"
            }
        }

        Item{
            Layout.preferredHeight:Kirigami.Units.largeSpacing
        }

        QQC2.CheckBox{
            id: chb_lookForOtherOs
            Kirigami.FormData.label: "Generated entries:"
            text: "Look for other Operating systems"
            checked: kcm.grubData.lookForOtherOs
            onClicked: {
                kcm.grubData.lookForOtherOs = checked
                kcm.settingsChanged()
                if (checked){
                    if (rb_immediately.checked){
                        kcm.grubData.timeout = 10
                    }
                    if(!chb_showMenu.checked){
                        kcm.grubData.timeoutStyle= "menu"
                    }
                }
            }
        }
    }
}
