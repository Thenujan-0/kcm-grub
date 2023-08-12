
set(KCM_GRUB_SRCS
    kcmgrub.cpp
    common.cpp
    entry.cpp
    grubdata.cpp
)
add_subdirectory(helper)
add_library(kcm_grub MODULE ${KCM_GRUB_SRCS})

target_compile_definitions(kcm_grub PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(kcm_grub
    KF5::I18n
    KF5::KCMUtils
    KF5::KIOGui
    KF5::Screen
    KF5::QuickAddons
    KF5::AuthCore
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KIOFileWidgets
    Qt5::Test
)

kcmutils_generate_desktop_file(kcm_grub)

add_subdirectory(tests)


kpackage_install_package(package kcm_grub kcms)


install(TARGETS kcm_grub DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings)

