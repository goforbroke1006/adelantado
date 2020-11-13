project(adelantado-tests)

add_executable(${PROJECT_NAME}
        main.cpp

        ../url.cpp url.test.cpp
        ../app.cpp app.test.cpp
        ../src/html/GumboPageScanner.cpp src/html/GumboPageScanner.test.cpp
        )

target_link_libraries(${PROJECT_NAME} gtest gtest_main pthread)

add_test(runUnitTests ${PROJECT_NAME})

# add google html parser
target_link_libraries(${PROJECT_NAME} gumbo)