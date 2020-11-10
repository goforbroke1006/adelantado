project(adelantado-tests)

add_executable(${PROJECT_NAME} main.cpp url.test.cpp)

target_link_libraries(${PROJECT_NAME} gtest gtest_main pthread)

add_test(runUnitTests ${PROJECT_NAME})
