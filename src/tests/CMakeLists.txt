include_directories(../)
link_directories(../)
include(ECMMarkAsTest)

set(CMAKE_CXX_STANDARD 17)

set(TEST_SRCS ../grubdata.cpp ./testutils.cpp ../entry.cpp ../common.cpp )
set(TEST_LIBS Qt::Test KF5::AuthCore KF5::KIOCore KF5::KIOWidgets stdc++fs)

set(TEST_NAME testsetvalue)

add_executable(${TEST_NAME} setvalue.cpp ${TEST_SRCS})
target_link_libraries(${TEST_NAME} ${TEST_LIBS})
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
ecm_mark_as_test(${TEST_NAME})

set(TEST_NAME testloadvalues)

add_executable(${TEST_NAME} loadvalues.cpp ${TEST_SRCS})
target_link_libraries(${TEST_NAME} ${TEST_LIBS})
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
ecm_mark_as_test(${TEST_NAME})