cmake_minimum_required(VERSION 3.10)

project(SearchEngineOnline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include /usr/local/include)
link_directories(/usr/local/lib)

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(search_server
    src/Codec.cpp
    src/KeywordRecommender.cpp
    src/SearchServer.cpp
    src/WebSearcher.cpp
    src/server_main.cpp
)

target_link_libraries(search_server
    muduo_net
    muduo_base
    pthread
    rt
)

add_executable(search_client
    src/Codec.cpp
    src/client_main.cpp
)

target_link_libraries(search_client
    muduo_net
    muduo_base
    pthread
    rt
)

add_executable(test_keyword
    src/KeywordRecommender.cpp
    src/test_keyword.cpp
)

add_executable(test_websearch
    src/WebSearcher.cpp
    src/test_websearch.cpp
)
