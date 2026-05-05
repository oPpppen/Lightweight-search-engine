#include "KeywordRecommender.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>

KeywordRecommender::KeywordRecommender(const std::string& dictFile,
                                       const std::string& indexFile)
{
    lineToWord_.push_back("");
    loadDict(dictFile);
    loadIndex(indexFile);

    const std::size_t englishLineOffset = lineToWord_.size() - 1;
    const std::string enDictFile = inferSiblingFile(dictFile, "ch_dict.dat", "en_dict.dat");
    const std::string enIndexFile = inferSiblingFile(indexFile, "ch_index.dat", "en_index.dat");

    std::ifstream ifsEnDict(enDictFile);
    std::ifstream ifsEnIndex(enIndexFile);
    if (ifsEnDict && ifsEnIndex) {
        loadDict(enDictFile);
        loadIndex(enIndexFile, englishLineOffset);
    }
}

std::string KeywordRecommender::recommend(const std::string& query, int k)
{
    if (query.empty()) {
        return buildJson("", {});
    }

    if (k <= 0) {
        return buildJson(query, {});
    }

    std::set<std::string> candidates;
    const auto chars = splitUtf8Chars(query);
    for (const auto& ch : chars) {
        auto it = index_.find(ch);
        if (it != index_.end()) {
            for (int lineNumber : it->second) {
                if (lineNumber <= 0 ||
                    static_cast<std::size_t>(lineNumber) >= lineToWord_.size()) {
                    continue;
                }

                const std::string& word = lineToWord_[static_cast<std::size_t>(lineNumber)];
                if (!word.empty()) {
                    candidates.insert(word);
                }
            }
        }
    }

    if (candidates.empty()) {
        return buildJson(query, {});
    }

    struct Candidate {
        std::string word;
        int distance;
        int frequency;
    };

    std::vector<Candidate> scored;
    scored.reserve(candidates.size());

    for (const auto& word : candidates) {
        int frequency = 0;
        auto dictIt = dict_.find(word);
        if (dictIt != dict_.end()) {
            frequency = dictIt->second;
        }

        scored.push_back({word, editDistance(query, word), frequency});
    }

    std::sort(scored.begin(), scored.end(),
              [](const Candidate& lhs, const Candidate& rhs) {
                  if (lhs.distance != rhs.distance) {
                      return lhs.distance < rhs.distance;
                  }
                  if (lhs.frequency != rhs.frequency) {
                      return lhs.frequency > rhs.frequency;
                  }
                  return lhs.word < rhs.word;
              });

    std::vector<std::string> result;
    const std::size_t limit = std::min<std::size_t>(static_cast<std::size_t>(k), scored.size());
    result.reserve(limit);
    for (std::size_t i = 0; i < limit; ++i) {
        result.push_back(scored[i].word);
    }

    return buildJson(query, result);
}

void KeywordRecommender::loadDict(const std::string& dictFile)
{
    std::ifstream ifs(dictFile);
    if (!ifs) {
        std::cerr << "failed to open dict file: " << dictFile << std::endl;
        return;
    }

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) {
            lineToWord_.push_back("");
            continue;
        }

        std::istringstream iss(line);
        std::string word;
        int frequency = 0;
        if (!(iss >> word >> frequency)) {
            lineToWord_.push_back("");
            continue;
        }

        lineToWord_.push_back(word);
        dict_[word] = frequency;
    }
}

void KeywordRecommender::loadIndex(const std::string& indexFile, std::size_t lineOffset)
{
    std::ifstream ifs(indexFile);
    if (!ifs) {
        std::cerr << "failed to open index file: " << indexFile << std::endl;
        return;
    }

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        std::string key;
        iss >> key;
        if (key.empty()) {
            continue;
        }

        int lineNumber = 0;
        while (iss >> lineNumber) {
            if (lineNumber <= 0) {
                continue;
            }
            index_[key].insert(static_cast<int>(lineOffset + static_cast<std::size_t>(lineNumber)));
        }
    }
}

std::string KeywordRecommender::inferSiblingFile(const std::string& path,
                                                 const std::string& fromName,
                                                 const std::string& toName) const
{
    const std::size_t pos = path.rfind(fromName);
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(0, pos) + toName;
}

std::vector<std::string> KeywordRecommender::splitUtf8Chars(const std::string& str) const
{
    std::vector<std::string> result;

    for (std::size_t i = 0; i < str.size();) {
        unsigned char byte = static_cast<unsigned char>(str[i]);
        std::size_t len = 1;

        if ((byte & 0x80) == 0x00) {
            len = 1;
        } else if ((byte & 0xE0) == 0xC0) {
            len = 2;
        } else if ((byte & 0xF0) == 0xE0) {
            len = 3;
        } else if ((byte & 0xF8) == 0xF0) {
            len = 4;
        }

        if (i + len > str.size()) {
            len = 1;
        }

        result.push_back(str.substr(i, len));
        i += len;
    }

    return result;
}

int KeywordRecommender::editDistance(const std::string& lhs, const std::string& rhs) const
{
    const auto left = splitUtf8Chars(lhs);
    const auto right = splitUtf8Chars(rhs);

    std::vector<std::vector<int>> dp(left.size() + 1,
                                     std::vector<int>(right.size() + 1, 0));

    for (std::size_t i = 0; i <= left.size(); ++i) {
        dp[i][0] = static_cast<int>(i);
    }
    for (std::size_t j = 0; j <= right.size(); ++j) {
        dp[0][j] = static_cast<int>(j);
    }

    for (std::size_t i = 1; i <= left.size(); ++i) {
        for (std::size_t j = 1; j <= right.size(); ++j) {
            if (left[i - 1] == right[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = std::min({dp[i - 1][j] + 1,
                                     dp[i][j - 1] + 1,
                                     dp[i - 1][j - 1] + 1});
            }
        }
    }

    return dp[left.size()][right.size()];
}

std::string KeywordRecommender::buildJson(const std::string& query,
                                          const std::vector<std::string>& words) const
{
    std::ostringstream oss;
    oss << "{\"type\":\"keyword\",\"query\":\"" << escapeJson(query) << "\",\"result\":[";

    for (std::size_t i = 0; i < words.size(); ++i) {
        if (i != 0) {
            oss << ",";
        }
        oss << "\"" << escapeJson(words[i]) << "\"";
    }

    oss << "]}";
    return oss.str();
}

std::string KeywordRecommender::escapeJson(const std::string& text) const
{
    std::string result;
    result.reserve(text.size());

    for (char ch : text) {
        switch (ch) {
        case '\\':
            result += "\\\\";
            break;
        case '"':
            result += "\\\"";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        default:
            result.push_back(ch);
            break;
        }
    }

    return result;
}
