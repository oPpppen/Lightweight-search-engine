find_path(TINYXML2_INCLUDE_DIR tinyxml2.h)
find_library(TINYXML2_LIBRARY tinyxml2)

if(NOT TINYXML2_INCLUDE_DIR OR NOT TINYXML2_LIBRARY)
    message(FATAL_ERROR "tinyxml2 headers or library not found")
endif()

add_executable(webpage_builder
    src/DirectoryScanner.cc
    src/PageProcessor.cc
    src/main.cc
)

target_include_directories(webpage_builder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    /usr/local/include
    ${TINYXML2_INCLUDE_DIR}
)

target_compile_definitions(webpage_builder PRIVATE
    SEARCH_ENGINE_JIEBA_DICT_DIR=\"/usr/local/dict\"
    SEARCH_ENGINE_WEBPAGE_STOPWORD_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/stopwords\"
    SEARCH_ENGINE_WEBPAGE_XML_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/webpages\"
    SEARCH_ENGINE_OUTPUT_DIR=\"${SEARCH_ENGINE_OUTPUT_DIR}\"
)

target_link_libraries(webpage_builder PRIVATE ${TINYXML2_LIBRARY})

set_target_properties(webpage_builder PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
