cmake_minimum_required(VERSION 3.16)

project(SearchEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SEARCH_ENGINE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/data")
file(MAKE_DIRECTORY "${SEARCH_ENGINE_OUTPUT_DIR}")

add_subdirectory(keyword)
add_subdirectory(webpage)
add_subdirectory(SearchEngineOnline)
