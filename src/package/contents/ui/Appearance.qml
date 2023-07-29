import org.kde.kirigami 2.14 as Kirigami
import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import org.kde.plasma.kcm.data 1.0

Kirigami.FormLayout{

    Kirigami.Separator {
        Kirigami.FormData.label: i18nc("@title:group", "Resolutions")
        Kirigami.FormData.isSection: true
    }

    QQC2.ComboBox{
        Kirigami.FormData.label:"GRUB"
    }
    QQC2.ComboBox{
        Kirigami.FormData.label:"Linux Kernel"

    }


}