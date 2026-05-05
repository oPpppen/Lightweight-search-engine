#include "WebSearcher.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>

namespace {

const char* kDictPath = "/usr/local/dict/jieba.dict.utf8";
const char* kHmmPath = "/usr/local/dict/hmm_model.utf8";
const char* kUserDictPath = "/usr/local/dict/user.dict.utf8";
const char* kIdfPath = "/usr/local/dict/idf.utf8";
const char* kJiebaStopPath = "/usr/local/dict/stop_words.utf8";

bool isBlankWord(const std::string& word)
{
    return std::all_of(word.begin(), word.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
}

} // namespace

WebSearcher::WebSearcher(const std::string& pageLibFile,
                         const std::string& offsetLibFile,
                         const std::string& invertedIndexFile,
                         const std::string& stopWordsFile)
    : pageLibFile_(pageLibFile)
    , tokenizer_(kDictPath, kHmmPath, kUserDictPath, kIdfPath, kJiebaStopPath)
{
    loadOffsetLib(offsetLibFile);
    loadInvertedIndex(invertedIndexFile);
    loadStopWords(stopWordsFile);
}

std::string WebSearcher::search(const std::string& query, int topN)
{
    nlohmann::json output;
    output["type"] = "search";
    output["query"] = query;
    output["result"] = nlohmann::json::array();

    if (query.empty() || topN <= 0) {
        return output.dump();
    }

    const std::vector<std::string> queryWords = cutQuery(query);
    if (queryWords.empty()) {
        return output.dump();
    }

    const auto queryVector = buildQueryVector(queryWords);
    const std::vector<int> candidateDocs = findCandidateDocs(queryWords);
    if (candidateDocs.empty()) {
        return output.dump();
    }

    struct ScoredDoc {
        int docId = 0;
        double score = 0.0;
    };

    std::vector<ScoredDoc> scoredDocs;
    scoredDocs.reserve(candidateDocs.size());
    for (int docId : candidateDocs) {
        scoredDocs.push_back({docId, cosineSimilarity(queryVector, docId)});
    }

    std::sort(scoredDocs.begin(), scoredDocs.end(),
              [](const ScoredDoc& lhs, const ScoredDoc& rhs) {
                  if (lhs.score != rhs.score) {
                      return lhs.score > rhs.score;
                  }
                  return lhs.docId < rhs.docId;
              });

    const std::size_t limit = std::min<std::size_t>(static_cast<std::size_t>(topN),
                                                    scoredDocs.size());
    for (std::size_t i = 0; i < limit; ++i) {
        const Document doc = readDocumentById(scoredDocs[i].docId);
        if (doc.id == 0) {
            continue;
        }

        nlohmann::json item;
        item["id"] = doc.id;
        item["title"] = doc.title;
        item["link"] = doc.link;
        item["abstract"] = makeAbstract(doc.content);
        output["result"].push_back(item);
    }

    return output.dump();
}

void WebSearcher::loadOffsetLib(const std::string& offsetLibFile)
{
    std::ifstream ifs(offsetLibFile);
    if (!ifs) {
        std::cerr << "failed to open offset lib file: " << offsetLibFile << std::endl;
        return;
    }

    int docId = 0;
    long offset = 0;
    long length = 0;
    while (ifs >> docId >> offset >> length) {
        offsetLib_[docId] = Offset{offset, length};
    }
}

void WebSearcher::loadInvertedIndex(const std::string& invertedIndexFile)
{
    std::ifstream ifs(invertedIndexFile);
    if (!ifs) {
        std::cerr << "failed to open inverted index file: "
                  << invertedIndexFile << std::endl;
        return;
    }

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        std::string word;
        iss >> word;
        if (word.empty()) {
            continue;
        }

        int docId = 0;
        double weight = 0.0;
        while (iss >> docId >> weight) {
            invertedIndex_[word][docId] = weight;
        }
    }
}

void WebSearcher::loadStopWords(const std::string& stopWordsFile)
{
    std::ifstream ifs(stopWordsFile);
    if (!ifs) {
        std::cerr << "failed to open stop words file: " << stopWordsFile << std::endl;
        return;
    }

    std::string word;
    while (ifs >> word) {
        stopWords_.insert(word);
    }
}

std::vector<std::string> WebSearcher::cutQuery(const std::string& query) const
{
    std::vector<std::string> words;
    tokenizer_.Cut(query, words);

    std::vector<std::string> result;
    for (const auto& word : words) {
        if (word.empty() || isBlankWord(word)) {
            continue;
        }
        if (stopWords_.find(word) != stopWords_.end()) {
            continue;
        }
        result.push_back(word);
    }

    return result;
}

std::unordered_map<std::string, double> WebSearcher::buildQueryVector(
    const std::vector<std::string>& words) const
{
    std::unordered_map<std::string, double> queryVector;
    for (const auto& word : words) {
        queryVector[word] += 1.0;
    }
    return queryVector;
}

std::vector<int> WebSearcher::findCandidateDocs(
    const std::vector<std::string>& queryWords) const
{
    if (queryWords.empty()) {
        return {};
    }

    std::vector<std::string> uniqueWords;
    std::set<std::string> seenWords;
    for (const auto& word : queryWords) {
        if (seenWords.insert(word).second) {
            uniqueWords.push_back(word);
        }
    }

    auto firstIt = invertedIndex_.find(uniqueWords.front());
    if (firstIt == invertedIndex_.end()) {
        return {};
    }

    std::set<int> currentDocs;
    for (const auto& entry : firstIt->second) {
        currentDocs.insert(entry.first);
    }

    for (std::size_t i = 1; i < uniqueWords.size() && !currentDocs.empty(); ++i) {
        auto it = invertedIndex_.find(uniqueWords[i]);
        if (it == invertedIndex_.end()) {
            return {};
        }

        std::set<int> nextDocs;
        for (const auto& entry : it->second) {
            nextDocs.insert(entry.first);
        }

        std::set<int> intersection;
        std::set_intersection(currentDocs.begin(), currentDocs.end(),
                              nextDocs.begin(), nextDocs.end(),
                              std::inserter(intersection, intersection.begin()));
        currentDocs = std::move(intersection);
    }

    return std::vector<int>(currentDocs.begin(), currentDocs.end());
}

double WebSearcher::cosineSimilarity(
    const std::unordered_map<std::string, double>& queryVector,
    int docId) const
{
    double dot = 0.0;
    double queryNormSquare = 0.0;
    double docNormSquare = 0.0;

    for (const auto& [word, queryWeight] : queryVector) {
        queryNormSquare += queryWeight * queryWeight;

        double docWeight = 0.0;
        auto indexIt = invertedIndex_.find(word);
        if (indexIt != invertedIndex_.end()) {
            auto docIt = indexIt->second.find(docId);
            if (docIt != indexIt->second.end()) {
                docWeight = docIt->second;
            }
        }

        dot += queryWeight * docWeight;
        docNormSquare += docWeight * docWeight;
    }

    const double denominator = std::sqrt(queryNormSquare) * std::sqrt(docNormSquare);
    if (denominator == 0.0) {
        return 0.0;
    }

    return dot / denominator;
}

WebSearcher::Document WebSearcher::readDocumentById(int docId) const
{
    Document doc;

    auto offsetIt = offsetLib_.find(docId);
    if (offsetIt == offsetLib_.end()) {
        std::cerr << "doc id not found in offset lib: " << docId << std::endl;
        return doc;
    }

    std::ifstream ifs(pageLibFile_);
    if (!ifs) {
        std::cerr << "failed to open page lib file: " << pageLibFile_ << std::endl;
        return doc;
    }

    ifs.seekg(offsetIt->second.offset);
    std::string raw(offsetIt->second.length, '\0');
    if (raw.empty()) {
        return doc;
    }
    ifs.read(&raw[0], static_cast<std::streamsize>(offsetIt->second.length));
    raw.resize(static_cast<std::size_t>(ifs.gcount()));

    doc.id = docId;
    const std::string idText = getTagValue(raw, "id");
    if (!idText.empty()) {
        doc.id = std::stoi(idText);
    }
    doc.link = getTagValue(raw, "link");
    doc.title = getTagValue(raw, "title");
    doc.content = getTagValue(raw, "content");

    return doc;
}

std::string WebSearcher::makeAbstract(const std::string& content,
                                      std::size_t maxChars) const
{
    const auto chars = splitUtf8Chars(content);
    if (chars.size() <= maxChars) {
        return content;
    }

    std::string result;
    for (std::size_t i = 0; i < maxChars; ++i) {
        result += chars[i];
    }
    result += "...";
    return result;
}

std::string WebSearcher::getTagValue(const std::string& doc,
                                     const std::string& tag) const
{
    const std::string beginTag = "<" + tag + ">";
    const std::string endTag = "</" + tag + ">";

    const std::size_t beginPos = doc.find(beginTag);
    if (beginPos == std::string::npos) {
        return "";
    }

    const std::size_t contentPos = beginPos + beginTag.size();
    const std::size_t endPos = doc.find(endTag, contentPos);
    if (endPos == std::string::npos) {
        return "";
    }

    return doc.substr(contentPos, endPos - contentPos);
}

std::vector<std::string> WebSearcher::splitUtf8Chars(const std::string& str) const
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
