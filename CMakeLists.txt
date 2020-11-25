cmake_minimum_required(VERSION 3.12)
project(adelantado)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} main.cpp
        src/url.h src/url.cpp
        src/HTTPClient.h src/HTTPClient.cpp
        src/runtime.h
        src/cfgloader.h
        src/html-filter.h src/html-filter.cpp
        src/app.h src/app.cpp

        src/parser/AbstractPageScanner.h
        src/parser/GumboPageScanner.h src/parser/GumboPageScanner.cpp
        src/parser/KeywordEntries.h src/parser/KeywordEntries.cpp

        src/storage/LinkStorage.h src/storage/LinkStorage.cpp

        src/Metrics.h src/Metrics.cpp
        )

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

# fmt
#target_link_libraries(${PROJECT_NAME} fmt)

# libconfig++-dev
target_link_libraries(${PROJECT_NAME} config++)

# prometheus-cpp
target_link_libraries(${PROJECT_NAME} prometheus-cpp-pull prometheus-cpp-core)
