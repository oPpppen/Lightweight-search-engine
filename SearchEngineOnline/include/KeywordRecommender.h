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
    void loadIndex(const std::string& indexFile);

    std::vector<std::string> splitUtf8Chars(const std::string& str) const;
    int editDistance(const std::string& lhs, const std::string& rhs) const;
    std::string buildJson(const std::string& query,
                          const std::vector<std::string>& words) const;
    std::string escapeJson(const std::string& text) const;

private:
    std::unordered_map<std::string, int> dict_;
    std::unordered_map<std::string, std::set<std::string>> index_;
};
