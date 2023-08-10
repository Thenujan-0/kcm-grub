import org.kde.kirigami 2.14 as Kirigami
import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import org.kde.plasma.kcm.data 1.0

Kirigami.FormLayout{

    Kirigami.Separator {
        Kirigami.FormData.label: i18nc("@title:group", "Default entry")
        Kirigami.FormData.isSection: true
    }
    RowLayout{
        Kirigami.FormData.label: "Default entry:"

        QQC2.RadioButton{
            id:rb_pre_defined
            // checked:kcm.grubData.defaultEntryType == DefaultEntry.Predefined
            Binding on checked { value: kcm.grubData.defaultEntryType == DefaultEntry.Predefined }
            text: "predefined"

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
            id: cob_os_entries
            model: kcm.grubData.osEntries
            textRole: "title"
            currentIndex: kcm.grubData.osEntries.indexOf(kcm.grubData.defaultEntry)

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

    Kirigami.Separator {
        Kirigami.FormData.label: i18nc("@title:group", "Timeout and Menu")
        Kirigami.FormData.isSection: true
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
            objectName:"rb_immediately"
            Layout.leftMargin: Kirigami.Units.largeSpacing * 3

            text:"Immediately"
            // Workaround for https://bugreports.qt.io/browse/QTBUG-30801

            opacity: automaticallyBootDefault.checked && !chb_lookForOtherOs.checked ? 1 : 0.5
            checked:kcm.grubData.immediateTimeout
            onClicked: {
                if (!(automaticallyBootDefault.checked && !chb_lookForOtherOs.checked)){
                    checked = false
                    return
                }
                if (checked) {
                    rb_after.checked = false;
                    kcm.grubData.immediateTimeout = true;
                    kcm.settingsChanged();
                }
            }

            Binding on checked {
                value: kcm.grubData.immediateTimeout
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
            checked: !kcm.grubData.immediateTimeout
            onClicked: {
                if(checked){
                    rb_immediately.checked=false
                    kcm.grubData.timeout = Number(timeout.text) > 0 ? Number(timeout.text) : 10
                    kcm.settingsChanged()
                }
            }

            Binding on checked {
                value:  !kcm.grubData.immediateTimeout
            }

        }

        DoubleSpinBox {
            id: timeout
            objectName:"dsb_timeout"

            text: kcm.grubData.timeout <= 0 ? "10" : kcm.grubData.timeout.toString() 
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
        objectName: "chb_showMenu"
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

     Kirigami.Separator {
        Kirigami.FormData.label: i18nc("@title:group", "Generated entries")
        Kirigami.FormData.isSection: true
    }

    QQC2.ComboBox{
        id:cob_languages
        model:kcm.grubData.languages
        textRole:"formattedName"
        Kirigami.FormData.label: "Language: "
        currentIndex:kcm.grubData.languages.indexOf(kcm.grubData.language)
        


        onActivated: {
            kcm.grubData.language = kcm.grubData.languages[cob_languages.currentIndex]
            kcm.settingsChanged()
        }
    }
    QQC2.CheckBox{
        id: chb_lookForOtherOs
        objectName:"chb_lookForOtherOs"
        text: "Look for other Operating Systems"
        checked: kcm.grubData.lookForOtherOs
        onClicked: {
            kcm.grubData.lookForOtherOs = checked
            kcm.settingsChanged()
            if (checked){
                if (rb_immediately.checked){
                    kcm.grubData.timeout = 10
                    kcm.grubData.immediateTimeout = false;
                }
                if(!chb_showMenu.checked){
                    kcm.grubData.timeoutStyle= "menu"
                }
            }
        }
    }

    QQC2.CheckBox{
        id: chb_generate_recovery
        objectName:"chb_generate_recovery"
        text: "Generate recovery entries"
        checked: kcm.grubData.generateRecoveryEntries
        onClicked: {
            kcm.grubData.generateRecoveryEntries = checked
            kcm.settingsChanged()
        }
    }
}
