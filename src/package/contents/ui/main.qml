

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
        
        Component.onCompleted: 
        {
            if (kcm.grubData.lookForOtherOs){
                errorMessage.type = Kirigami.MessageType.Warning
                errorMessage.visible = true
                errorMessage.text = i18nd("kcm_grub2","<html><style type=\"text/css\"></style>As you have \"look for other operating systems\" enabled,"+ 
                    " Your menu related settings (timeout , show behavior) might get overridden</html>");
            }

                errorMessage.actions = [fixItAction]
            }
    }

    Kirigami.OverlaySheet{
        id:sheetOverrideWarning
        header:QQC2.Label{
            text:"Details on override behavior of \"Look for other OS\""
        }
        QQC2.Label{
            text:"If you have \"look for other os\" enabled, It will override some of your settings."+
                "\nThis is a feature to avoid the user from getting into a state where he cannot change grub settings"+
                "\nIf you have set the timeout before showing the menu to 0, It will automatically set it to 10" +
                "\nIf you have set a timeout before showing menu (hide menu for), It will be removed"
            wrapMode:Qt.WordWrap

        }
        footer:RowLayout{
            QQC2.Button{
                text: "Ok"
                onClicked: {
                    sheetOverrideWarning.close()
                }
            }
        }
    }

    Kirigami.FormLayout{
        RowLayout{
            Kirigami.FormData.label:"default entry:"

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

        RowLayout{

            Kirigami.FormData.label:"Timeout:"

            QQC2.CheckBox{
                id:chb_bootDefault

                text:"Hide the menu for"
                checked:kcm.grubData.hiddenTimeout !=0
            }

            DoubleSpinBox{
                id:grubTimeout
                text: kcm.grubData.hiddenTimeout
                onIncrease : function(){
                    kcm.grubData.hiddenTimeout +=1
                }
                onDecrease: function(){
                    kcm.grubData.hiddenTimeout -=1
                }

            }
            

            QQC2.Label{
                text:"seconds"
            }
        }
        RowLayout{//Just for indentation
            QQC2.CheckBox{
                Layout.leftMargin: Kirigami.Units.largeSpacing *3
                text: "Show countdown timer"
                checked: kcm.grubData.timeoutStyle == "countdown"
                onClicked: {
                    if (!chb_bootDefault.checked){
                        return
                    }
                    if(checked){
                        kcm.grubData.timeoutStyle="countdown"
                    }else{
                        kcm.grubData.timeoutStyle="hidden"
                    }
                }
            }
        }
        Item{
            Layout.preferredHeight:Kirigami.Units.largeSpacing
        }

        QQC2.CheckBox{
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
                id:immediately
                Layout.leftMargin: Kirigami.Units.largeSpacing *3
                text:"Immediately"
                enabled:automaticallyBootDefault.checked

                checked:kcm.grubData.timeout == 0
                onClicked: {
                    if(checked){
                        after.checked=false
                        kcm.grubData.timeout = 0
                        kcm.settingsChanged()
                    }
                }
            }
        }
        RowLayout{
            QQC2.RadioButton{
                id:after
                Layout.leftMargin: Kirigami.Units.largeSpacing *3
                text:"After"
                enabled:automaticallyBootDefault.checked
                checked:kcm.grubData.timeout != 0
                onClicked: {
                    if(checked){
                        immediately.checked=false
                        kcm.grubData.timeout = Number(timeout.text)
                        kcm.settingsChanged()
                    }
                }


            }
            DoubleSpinBox{
                id:timeout
                text: kcm.grubData.timeout.toString()
                onIncrease: function(){ kcm.grubData.timeout += 1 }
                onDecrease: function(){ kcm.grubData.timeout -= 1 }

                onEditFinished: {
                    if(after.checked){
                        kcm.grubData.timeout = Number(text)
                        kcm.settingsChanged()
                    }
                }
            }

            QQC2.Label{
                text:"seconds"
            }
        }

        Item{
            Layout.preferredHeight:Kirigami.Units.largeSpacing
        }

        QQC2.CheckBox{
            Kirigami.FormData.label: "Generated entries:"
            text: "Look for other Operating systems"
            checked: kcm.grubData.lookForOtherOs
            onClicked: {
                kcm.grubData.lookForOtherOs = checked
                kcm.settingsChanged()
            }
        }
    }
}
