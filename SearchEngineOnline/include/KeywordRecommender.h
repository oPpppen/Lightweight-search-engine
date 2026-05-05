#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class KeywordRecommender {
public:
    KeywordRecommender(const std::string& dictFile,
                       const std::string& indexFile);

    std::string recommend(const std::string& query, int k = 5);

private:
    void loadDict(const std::string& dictFile);
    void loadIndex(const std::string& indexFile, std::size_t lineOffset = 0);
    std::string inferSiblingFile(const std::string& path,
                                 const std::string& fromName,
                                 const std::string& toName) const;

    std::vector<std::string> splitUtf8Chars(const std::string& str) const;
    int editDistance(const std::string& lhs, const std::string& rhs) const;
    std::string buildJson(const std::string& query,
                          const std::vector<std::string>& words) const;
    std::string escapeJson(const std::string& text) const;

private:
    std::unordered_map<std::string, int> dict_;
    std::vector<std::string> lineToWord_;
    std::unordered_map<std::string, std::set<int>> index_;
};
