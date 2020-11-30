project(adelantado-tests)

add_executable(${PROJECT_NAME}
        main.cpp

        ../src/url.cpp url.test.cpp
        ../src/char-conv.h char-conv.test.cpp

        src/scrape/VectorBulkSplitter.test.cpp
        ../src/scrape/utils.h src/scrape/utils.test.cpp
        ../src/parser/GumboPageScanner.cpp src/parser/GumboPageScanner.test.cpp
        ../src/parser/KeywordEntries.cpp src/parser/KeywordEntries.test.cpp
        )

target_link_libraries(${PROJECT_NAME} gtest gtest_main pthread)

add_test(runUnitTests ${PROJECT_NAME})

# add google html parser
target_link_libraries(${PROJECT_NAME} gumbo)