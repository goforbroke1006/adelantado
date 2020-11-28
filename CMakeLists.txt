cmake_minimum_required(VERSION 3.12)
project(adelantado)

set(CMAKE_CXX_STANDARD 14)

include_directories(
        modules/goforbroke1006/goxx-std
        modules/nlohmann/json/single_include
)
set(SOURCES  src/url.h src/url.cpp
        src/HTTPClient.h src/HTTPClient.cpp
        src/runtime.h
        src/cfgloader.h
        src/html-filter.h src/html-filter.cpp

        src/parser/AbstractPageScanner.h
        src/parser/GumboPageScanner.h src/parser/GumboPageScanner.cpp
        src/parser/KeywordEntries.h src/parser/KeywordEntries.cpp

        src/storage/LinkStorage.h src/storage/LinkStorage.cpp

        src/Metrics.h src/Metrics.cpp
        src/scrape/MultiThreadPageScrapper.h src/scrape/MultiThreadPageScrapper.cpp
        src/scrape/ObserverResult.h src/scrape/ObserverResult.cpp
        src/scrape/VectorBulkSplitter.h)


include_directories(modules/goforbroke1006/cpp-logger)
set(SOURCES ${SOURCES} modules/goforbroke1006/cpp-logger/logger.cpp)


add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

find_package(CURL)
if (CURL_FOUND)
    INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
else (CURL_FOUND)
    MESSAGE(FATAL_ERROR "Could not find the CURL library and development files.")
endif (CURL_FOUND)


#enable_testing()
add_subdirectory(tests)


# move configs
file(COPY adelantado.cfg DESTINATION ${CMAKE_BINARY_DIR})
file(COPY keywords.txt DESTINATION ${CMAKE_BINARY_DIR})
file(COPY links.txt DESTINATION ${CMAKE_BINARY_DIR})
file(COPY keyword-ignore.txt DESTINATION ${CMAKE_BINARY_DIR})
file(COPY domain-priority.txt DESTINATION ${CMAKE_BINARY_DIR})

# add google html parser
target_link_libraries(${PROJECT_NAME} gumbo)

# postgres client
target_link_libraries(${PROJECT_NAME} pq)

# pthread
target_link_libraries(${PROJECT_NAME} pthread)

# libconfig++-dev
target_link_libraries(${PROJECT_NAME} config++)

# prometheus-cpp
target_link_libraries(${PROJECT_NAME} prometheus-cpp-pull prometheus-cpp-core)
