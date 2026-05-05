#include "../include/KeywordRecommender.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    std::string dictFile = "data/ch_dict.dat";
    std::string indexFile = "data/ch_index.dat";
    std::string query = "中国";

    if (argc >= 3) {
        dictFile = argv[1];
        indexFile = argv[2];
    }
    if (argc >= 4) {
        query = argv[3];
    }

    KeywordRecommender recommender(dictFile, indexFile);
    std::cout << recommender.recommend(query, 5) << std::endl;
    return 0;
}
