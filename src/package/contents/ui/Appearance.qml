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
    Kirigami.Separator {
        Kirigami.FormData.label: i18nc("@title:group", "Colors")
        Kirigami.FormData.isSection: true
    }

    QQC2.Label {
        text: i18n("These colors are used by normal and wallpaper modes only")
    }

    readonly property var colors: [ 
        {text: i18nc("@item:inlistbox Refers to color.", "Black"),  color: "black",  value: "black"},
        {text: i18nc("@item:inlistbox Refers to color.", "Blue"),  color: "blue",  value: "blue"},
        {text: i18nc("@item:inlistbox Refers to color.", "Cyan"), color: "cyan", value: "cyan"},
        {text: i18nc("@item:inlistbox Refers to color.", "Dark Gray"), color: "darkgray", value: "dark-gray"},
        {text: i18nc("@item:inlistbox Refers to color.", "Green"), color: "green", value: "green"},
        {text: i18nc("@item:inlistbox Refers to color.", "Light Cyan"), color: "light-cyan", value: "lightcyan"},
        {text: i18nc("@item:inlistbox Refers to color.", "Light Blue"), color: "lightblue", value: "light-blue"},
        {text: i18nc("@item:inlistbox Refers to color.", "Light Green"), color: "lightgreen", value: "light-green"},
        {text: i18nc("@item:inlistbox Refers to color.", "Light Gray"), color: "lightgray", value: "light-gray"},
        {text: i18nc("@item:inlistbox Refers to color.", "Light Magenta"), color: "#ff80ff", value: "light-magenta"},
        {text: i18nc("@item:inlistbox Refers to color.", "Light Red"), color: "lightred", value: "light-red"},
        {text: i18nc("@item:inlistbox Refers to color.", "Magenta"), color: "magenta", value: "magenta"},
        {text: i18nc("@item:inlistbox Refers to color.", "Red"), color: "red", value: "red"},
        {text: i18nc("@item:inlistbox Refers to color.", "White"), color: "white", value: "white"},
        {text: i18nc("@item:inlistbox Refers to color.", "Yellow"), color: "yellow", value: "yellow"},
    ]
    function findColorIndex(colorValue) {
        var colorIndex;
        colors.forEach((color, index) => {
            if (color.value === colorValue) {
                colorIndex = index
            }
        })
        return colorIndex;
    }
    ColorComboBox {
        Kirigami.FormData.label: i18n("Normal Foreground (Text): ")
        model: colors
        currentIndex: findColorIndex(kcm.grubData.colorNormalForeground)
    
        onActivated: {
            kcm.grubData.colorNormalForeground = colors[currentIndex].value
            kcm.settingsChanged()
        }
    }
    ColorComboBox {
        Kirigami.FormData.label: i18n("Highlight Foreground (Text): ")
        model: colors
        currentIndex: findColorIndex(kcm.grubData.colorHighlightForeground)
        onActivated: {
            kcm.grubData.colorHighlightForeground = colors[currentIndex].value
            kcm.settingsChanged()
        }
    }
    ColorComboBox {
        Kirigami.FormData.label: i18n("Normal Background: ")
        model: colors
        currentIndex: findColorIndex(kcm.grubData.colorNormalBackground)
        onActivated: {
            kcm.grubData.colorNormalBackground = colors[currentIndex].value
            kcm.settingsChanged()
        }
    }
    ColorComboBox {
        Kirigami.FormData.label: i18n("Highlight Background: ")
        model: colors
        currentIndex: findColorIndex(kcm.grubData.colorHighlightBackground)
        onActivated: {
            kcm.grubData.colorHighlightBackground = colors[currentIndex].value
            kcm.settingsChanged()
        }
    }
    
}
