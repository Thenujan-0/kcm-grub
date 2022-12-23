
set(KCM_GRUB2_SRCS
    kcmgrub2.cpp
    common.cpp
    entry.cpp
    grubdata.cpp
)


add_library(kcm_grub2 MODULE ${KCM_GRUB2_SRCS})

kcmutils_generate_desktop_file(kcm_grub2)

target_compile_definitions(kcm_grub2 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(kcm_grub2
    KF5::I18n
    KF5::KCMUtils
    KF5::KIOGui
    KF5::NewStuff
    KF5::QuickAddons
)

kpackage_install_package(package kcm_grub2 kcms)


install(TARGETS kcm_grub2 DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings)

