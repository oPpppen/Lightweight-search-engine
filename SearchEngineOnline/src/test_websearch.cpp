#include "WebSearcher.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    std::string pageLibFile = "data/pagelib.dat";
    std::string offsetFile = "data/offset.dat";
    std::string invertedFile = "data/inverted_index.dat";
    std::string stopWordsFile = "data/stop_words.txt";
    std::string query = "人工智能 机器学习";

    if (argc >= 5) {
        pageLibFile = argv[1];
        offsetFile = argv[2];
        invertedFile = argv[3];
        stopWordsFile = argv[4];
    }
    if (argc >= 6) {
        query = argv[5];
    }

    WebSearcher searcher(pageLibFile, offsetFile, invertedFile, stopWordsFile);
    std::cout << searcher.search(query, 10) << std::endl;
    return 0;
}
