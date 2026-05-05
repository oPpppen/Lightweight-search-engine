add_executable(keyword_builder
    src/DirectoryScanner.cc
    src/KeywordProcessor.cc
    src/main.cc
)

target_include_directories(keyword_builder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    /usr/local/include
)

target_compile_definitions(keyword_builder PRIVATE
    SEARCH_ENGINE_JIEBA_DICT_DIR=\"/usr/local/dict\"
    SEARCH_ENGINE_KEYWORD_STOPWORD_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/stopwords\"
    SEARCH_ENGINE_KEYWORD_CN_CORPUS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/corpus/CN\"
    SEARCH_ENGINE_KEYWORD_EN_CORPUS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/corpus/EN\"
    SEARCH_ENGINE_OUTPUT_DIR=\"${SEARCH_ENGINE_OUTPUT_DIR}\"
)

set_target_properties(keyword_builder PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
