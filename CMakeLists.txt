cmake_minimum_required(VERSION 3.16)
project(adelantado)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp
        url.h url.cpp
        HTTPClient.h HTTPClient.cpp
        runtime.h
        cfgloader.h
        html-filter.h html-filter.cpp
        repo.h repo.cpp
        app.h app.cpp

        src/html/AbstractPageScanner.h
        src/html/GumboPageScanner.h src/html/GumboPageScanner.cpp
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
file(COPY keywords.txt DESTINATION ${CMAKE_BINARY_DIR})
file(COPY links.txt DESTINATION ${CMAKE_BINARY_DIR})

# add google html parser
target_link_libraries(${PROJECT_NAME} gumbo)

# postgres client
target_link_libraries(${PROJECT_NAME} pq)

# postgres client
target_link_libraries(${PROJECT_NAME} pthread)

# postgres client
target_link_libraries(${PROJECT_NAME} fmt)
