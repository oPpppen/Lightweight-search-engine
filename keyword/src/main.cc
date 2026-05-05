#include <iostream>
#include <string>

#include "KeywordProcessor.h"

#ifndef SEARCH_ENGINE_JIEBA_DICT_DIR
#define SEARCH_ENGINE_JIEBA_DICT_DIR "/usr/local/dict"
#endif

#ifndef SEARCH_ENGINE_KEYWORD_STOPWORD_DIR
#define SEARCH_ENGINE_KEYWORD_STOPWORD_DIR "keyword/stopwords"
#endif

#ifndef SEARCH_ENGINE_KEYWORD_CN_CORPUS_DIR
#define SEARCH_ENGINE_KEYWORD_CN_CORPUS_DIR "keyword/corpus/CN"
#endif

#ifndef SEARCH_ENGINE_KEYWORD_EN_CORPUS_DIR
#define SEARCH_ENGINE_KEYWORD_EN_CORPUS_DIR "keyword/corpus/EN"
#endif

#ifndef SEARCH_ENGINE_OUTPUT_DIR
#define SEARCH_ENGINE_OUTPUT_DIR "data"
#endif

int main(int argc, char *argv[]) {
  const std::string chDir =
      argc > 1 ? argv[1] : SEARCH_ENGINE_KEYWORD_CN_CORPUS_DIR;
  const std::string enDir =
      argc > 2 ? argv[2] : SEARCH_ENGINE_KEYWORD_EN_CORPUS_DIR;
  const std::string outputDir =
      argc > 3 ? argv[3] : SEARCH_ENGINE_OUTPUT_DIR;

  KeyWordProcessor processor(SEARCH_ENGINE_JIEBA_DICT_DIR,
                             SEARCH_ENGINE_KEYWORD_STOPWORD_DIR, outputDir);

  if (!processor.process(chDir, enDir)) {
    std::cerr << "keyword_builder failed." << std::endl;
    return 1;
  }

  return 0;
}
