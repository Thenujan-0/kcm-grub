import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.14 as Kirigami
import org.kde.plasma.kcm.data 1.0

Kirigami.FormLayout {
    Kirigami.Separator {
        Kirigami.FormData.label: i18nc("@title:group", "Resolutions")
        Kirigami.FormData.isSection: true
    }

    QQC2.ComboBox {
        function getCurrentIndex() {
            var resolutionsList = kcm.grubData.grubResolutions;
            for (var i = 0; i < resolutionsList.length; i++) {
                if (resolutionsList[i].value == kcm.grubData.grubResolution)
                    return i;
            }
        }

        Kirigami.FormData.label: "GRUB: "
        model: kcm.grubData.grubResolutions
        // Fallback value added because UI will be loaded before all resolutions are added
        currentIndex: getCurrentIndex() || 0
        textRole: "name"
        displayText: currentValue.name
        implicitWidth: Kirigami.Units.largeSpacing * 20
        onActivated: {
            kcm.grubData.grubResolution = kcm.grubData.grubResolutions[currentIndex].value;
            kcm.settingsChanged();
        }
    }

    QQC2.ComboBox {
        function getCurrentIndex() {
            var resolutionsList = kcm.grubData.linuxKernelResolutions;

            for (var i = 0; i < resolutionsList.length; i++) {
                if (resolutionsList[i].value == kcm.grubData.linuxKernelResolution)
                    return i;

            }
        }

        Kirigami.FormData.label: "Linux Kernel: "
        model: kcm.grubData.linuxKernelResolutions
        textRole: "name"
        // Fallback value added because UI will be loaded before all resolutions are added
        currentIndex: getCurrentIndex() || 0
        displayText: currentValue.name
        implicitWidth: Kirigami.Units.largeSpacing * 25
        onActivated: {
            kcm.grubData.linuxKernelResolution = kcm.grubData.linuxKernelResolutions[currentIndex].value;
            kcm.settingsChanged();
        }
    }
}
