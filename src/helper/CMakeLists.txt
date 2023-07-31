
# Set Sources
set(kcmgrubhelper_SRCS helper.cpp ../common.h ../common.cpp)

# Build & Link
add_executable(kcmgrubhelper ${kcmgrubhelper_SRCS})
target_link_libraries(kcmgrubhelper KF5::AuthCore KF5::I18n Qt5::DBus)

# Install
install(TARGETS kcmgrubhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(kcmgrubhelper org.kde.kcontrol.kcmgrub root)
kauth_install_actions(org.kde.kcontrol.kcmgrub kcmgrub.actions)

target_link_libraries(kcmgrubhelper KF5::KIOCore KF5::KIOFileWidgets KF5::KIOWidgets KF5::KIONTLM)