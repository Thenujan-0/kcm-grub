

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
        type: Kirigami.MessageType.Error
        showCloseButton: true
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
                    kcm.grubData.defaultEntry = cob_os_entries.currentText;
                    console.log(cob_os_entries.currentText,kcm.grubData.defaultEntry)
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
                checked: false
                onClicked: {
                    // kcm.grubData.set()
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
