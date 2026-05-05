#pragma once

#include <cppjieba/Jieba.hpp>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class WebSearcher {
public:
    WebSearcher(const std::string& pageLibFile,
                const std::string& offsetLibFile,
                const std::string& invertedIndexFile,
                const std::string& stopWordsFile);

    std::string search(const std::string& query, int topN = 10);

private:
    struct Offset {
        long offset = 0;
        long length = 0;
    };

    struct Document {
        int id = 0;
        std::string link;
        std::string title;
        std::string content;
    };

private:
    void loadOffsetLib(const std::string& offsetLibFile);
    void loadInvertedIndex(const std::string& invertedIndexFile);
    void loadStopWords(const std::string& stopWordsFile);

    std::vector<std::string> cutQuery(const std::string& query) const;
    std::unordered_map<std::string, double> buildQueryVector(
        const std::vector<std::string>& words) const;
    std::vector<int> findCandidateDocs(
        const std::vector<std::string>& queryWords) const;
    double cosineSimilarity(
        const std::unordered_map<std::string, double>& queryVector,
        int docId) const;
    Document readDocumentById(int docId) const;
    std::string makeAbstract(const std::string& content,
                             std::size_t maxChars = 80) const;
    std::string getTagValue(const std::string& doc,
                            const std::string& tag) const;
    std::vector<std::string> splitUtf8Chars(const std::string& str) const;

private:
    std::string pageLibFile_;
    std::unordered_map<int, Offset> offsetLib_;
    std::unordered_map<std::string, std::unordered_map<int, double>> invertedIndex_;
    std::unordered_map<int, double> docNorms_;
    std::unordered_set<std::string> stopWords_;
    cppjieba::Jieba tokenizer_;
};
