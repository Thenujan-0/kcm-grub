include_directories(../)
link_directories(../)
include(ECMMarkAsTest)

set(CMAKE_CXX_STANDARD 17)

set(TEST_SRCS ../grubdata.cpp ./testutils.cpp ../entry.cpp ../common.cpp )
set(TEST_LIBS stdc++fs 
    Qt5::QuickTest 
    Qt5::Qml 
    Qt5::Core 
    Qt5::Gui
    Qt5::Widgets 
    Qt5::Quick 
    Qt5::QuickWidgets
    Qt5::Test
    KF5::KCMUtils
    KF5::I18n
    KF5::KCMUtils
    KF5::KIOGui
    KF5::NewStuff
    KF5::QuickAddons
    KF5::AuthCore
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KIOFileWidgets
    KF5::Screen
)

set(TEST_NAME testsetvalue)

add_executable(${TEST_NAME} setvalue.cpp ${TEST_SRCS})
target_link_libraries(${TEST_NAME} ${TEST_LIBS})
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
target_include_directories(${TEST_NAME} PUBLIC BEFORE ${SRC_BINARY_DIR})
ecm_mark_as_test(${TEST_NAME})

set(TEST_NAME testloadvalues)

add_executable(${TEST_NAME} loadvalues.cpp ${TEST_SRCS})
target_link_libraries(${TEST_NAME} ${TEST_LIBS})
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
target_include_directories(${TEST_NAME} PUBLIC BEFORE ${SRC_BINARY_DIR})
ecm_mark_as_test(${TEST_NAME})

set(TEST_NAME testinvaliddefault)

add_executable(${TEST_NAME} invaliddefault.cpp ${TEST_SRCS})
target_link_libraries(${TEST_NAME} ${TEST_LIBS})
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
target_include_directories(${TEST_NAME} PUBLIC BEFORE ${SRC_BINARY_DIR})
ecm_mark_as_test(${TEST_NAME})

set(TEST_NAME guitest)
add_executable(${TEST_NAME} gui.cpp kcm.cpp ${TEST_SRCS})
target_link_libraries(${TEST_NAME} ${TEST_LIBS})
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
target_include_directories(${TEST_NAME} PUBLIC BEFORE ${SRC_BINARY_DIR})
ecm_mark_as_test(${TEST_NAME})