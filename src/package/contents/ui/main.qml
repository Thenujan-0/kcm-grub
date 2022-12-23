

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
        QQC2.ComboBox{
            Kirigami.FormData.label:"Defailt entry:"
            model: Data.osEntries
        }
    }

}
