project(kcm-grub2)
cmake_minimum_required(VERSION 3.16)
set(PROJECT_VERSION "5.26.80")

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.98.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
set(CMAKE_CXX_STANDARD 17)


find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(FeatureSummary)
include(KDEGitCommitHooks)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets Quick QuickWidgets DBus Test QuickTest)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    XmlGui
    Auth
    ConfigWidgets
    KIO
    Archive
    NewStuff
    Declarative
    KCMUtils
)
find_package(KF5Screen REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
include(GRUBPaths)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050c00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054200)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_grub\")
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)

# kauth_install_actions(org.kde.kcontrol.kcmtest kcm_test.actions)

# add_executable(kcmtest_authhelper testauthhelper.cpp)
# target_link_libraries(kcmtest_authhelper KF5::AuthCore KF5::ConfigCore KF5::Archive KF5::I18n)

# kauth_install_helper_files(kcmtest_authhelper org.kde.kcontrol.kcmtest root)
# install(TARGETS kcmtest_authhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)

# used to import config.h for the test files
set(SRC_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/src")

enable_testing ()
add_subdirectory(src)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)


