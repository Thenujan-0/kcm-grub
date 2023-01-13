

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.0


import org.kde.kcm 1.4 as KCM
import org.kde.kirigami 2.14 as Kirigami
import org.kde.kcms.grub2 1.0

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
                checked:false
                text:"predefined"

                onClicked: {
                    if(checked){
                        rb_previously_booted.checked=false
                    }
                }

            }
            QQC2.ComboBox{
                id:cob_os_entries
                model:Data.osEntries
            }
        }

        QQC2.RadioButton{
            id:rb_previously_booted
            text:"previously booted entry"
            checked:false
            onClicked: {
                if(checked){
                    rb_pre_defined.checked=false
                }
            }
        }

        Item{
            Layout.preferredHeight:Kirigami.Units.largeSpacing
        }

        RowLayout{

            Kirigami.FormData.label:"Visibility:"

            QQC2.CheckBox{
                id:chb_bootDefault

                text:"Boot default entry after"
                checked:Data.bootDefault
            }

            QQC2.TextField{
                id:grubTimeout
                text: Data.grubTimeout.toString()
            }
            QQC2.Button{
                icon.name:"add"
                onClicked: {
                    var val =parseFloat(grubTimeout.text) +1
                    grubTimeout.text = val.toString()
                }
            }
            QQC2.Button{
                icon.name:"remove"
                onClicked: {
                    var val =parseFloat(grubTimeout.text) -1
                    grubTimeout.text = val.toString()
                }
            }

            QQC2.Label{
                text:"seconds"
            }
        }

        QQC2.CheckBox{
            text: "Show menu"
            checked: Data.showMenu
        }

        Item{
            Layout.preferredHeight:Kirigami.Units.largeSpacing
        }

        QQC2.CheckBox{
            Kirigami.FormData.label: "Other:"
            text: "Look for other Operating systems"
        }
    }

}
