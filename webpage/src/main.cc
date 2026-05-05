#include <iostream>
#include <string>

#include "PageProcessor.h"

#ifndef SEARCH_ENGINE_JIEBA_DICT_DIR
#define SEARCH_ENGINE_JIEBA_DICT_DIR "/usr/local/dict"
#endif

#ifndef SEARCH_ENGINE_WEBPAGE_STOPWORD_DIR
#define SEARCH_ENGINE_WEBPAGE_STOPWORD_DIR "webpage/stopwords"
#endif

#ifndef SEARCH_ENGINE_WEBPAGE_XML_DIR
#define SEARCH_ENGINE_WEBPAGE_XML_DIR "webpage/webpages"
#endif

#ifndef SEARCH_ENGINE_OUTPUT_DIR
#define SEARCH_ENGINE_OUTPUT_DIR "data"
#endif

int main(int argc, char *argv[]) {
  const std::string xmlDir =
      argc > 1 ? argv[1] : SEARCH_ENGINE_WEBPAGE_XML_DIR;
  const std::string outputDir =
      argc > 2 ? argv[2] : SEARCH_ENGINE_OUTPUT_DIR;

  PageProcessor processor(SEARCH_ENGINE_JIEBA_DICT_DIR,
                          SEARCH_ENGINE_WEBPAGE_STOPWORD_DIR, outputDir);
  if (!processor.process(xmlDir)) {
    std::cerr << "webpage_builder failed." << std::endl;
    return 1;
  }

  return 0;
}
