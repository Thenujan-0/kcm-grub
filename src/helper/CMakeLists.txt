
# Set Sources
set(kcmgrub2helper_SRCS helper.cpp)


# Build & Link
add_executable(kcmgrub2helper ${kcmgrub2helper_SRCS})
target_link_libraries(kcmgrub2helper KF5::AuthCore KF5::I18n)

# Install
install(TARGETS kcmgrub2helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(kcmgrub2helper org.kde.kcontrol.kcmgrub2 root)
kauth_install_actions(org.kde.kcontrol.kcmgrub2 kcmgrub2.actions)

