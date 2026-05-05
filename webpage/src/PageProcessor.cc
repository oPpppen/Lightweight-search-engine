#include "PageProcessor.h"
#include "DirectoryScanner.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <tinyxml2.h>
#include <utfcpp/utf8.h>

namespace {

bool hash_exists(const std::vector<uint64_t> &simhashes, uint64_t h) {
  using namespace simhash;

  for (const auto &h1 : simhashes) {
    if (Simhasher::isEqual(h, h1)) {
      return true;
    }
  }
  return false;
}

// 判断一个 Unicode 码点是否为中文字符
bool chinese_character(uint32_t codepoint) {
  return (codepoint >= 0x4E00 && codepoint <= 0x9FFF) ||   // 基本汉字
         (codepoint >= 0x3400 && codepoint <= 0x4DBF) ||   // 扩展A
         (codepoint >= 0x20000 && codepoint <= 0x2A6DF) || // 扩展B
         (codepoint >= 0x2A700 && codepoint <= 0x2B73F) || // 扩展C
         (codepoint >= 0x2B740 && codepoint <= 0x2B81F) || // 扩展D
         (codepoint >= 0x2B820 && codepoint <= 0x2CEAF) || // 扩展E
         (codepoint >= 0x2CEB0 && codepoint <= 0x2EBEF) || // 扩展F
         (codepoint >= 0x30000 && codepoint <= 0x3134F) || // 扩展G
         (codepoint >= 0x31350 && codepoint <= 0x323AF) || // 扩展H
         (codepoint >= 0xF900 && codepoint <= 0xFAFF) ||   // 兼容汉字
         (codepoint >= 0x2F800 && codepoint <= 0x2FA1F); // 表意文字补充区
}

// 判断一个字符串是否全部由中文字符组成
bool chinese_word(const std::string &word) {
  auto it = utf8::iterator<std::string::const_iterator>(
      word.begin(), word.begin(), word.end());
  auto end = utf8::iterator<std::string::const_iterator>(
      word.end(), word.begin(), word.end());
  for (; it != end; ++it) {
    if (!chinese_character(*it)) {
      return false;
    }
  }
  return true;
}

std::string build_jieba_path(const std::string &dir,
                             const std::string &filename) {
  if (dir.empty()) {
    return filename;
  }
  if (dir.back() == '/') {
    return dir + filename;
  }
  return dir + "/" + filename;
}

std::string extract_clean_text(const tinyxml2::XMLElement *element,
                               const std::regex &tagRegex) {
  if (element == nullptr) {
    return "";
  }

  const char *text = element->GetText();
  if (text == nullptr) {
    return "";
  }

  return std::regex_replace(std::string(text), tagRegex, "");
}

} // namespace

//加载所有停用词
PageProcessor::PageProcessor(const std::string &jiebaDictDir,
                             const std::string &stopwordDir,
                             const std::string &outputDir)
    : tokenizer_(build_jieba_path(jiebaDictDir, "jieba.dict.utf8"),
                 build_jieba_path(jiebaDictDir, "hmm_model.utf8"),
                 build_jieba_path(jiebaDictDir, "user.dict.utf8"),
                 build_jieba_path(jiebaDictDir, "idf.utf8"),
                 build_jieba_path(jiebaDictDir, "stop_words.utf8")),
      hasher_(build_jieba_path(jiebaDictDir, "jieba.dict.utf8"),
              build_jieba_path(jiebaDictDir, "hmm_model.utf8"),
              build_jieba_path(jiebaDictDir, "idf.utf8"),
              build_jieba_path(jiebaDictDir, "stop_words.utf8")),
      outputDir_(outputDir) {
  std::filesystem::create_directories(outputDir_);

  //获取所有文件的名称
  std::vector<std::string> files = DirectoryScanner::scan(stopwordDir);
  if (files.empty()) {
    std::cerr << "No stopword files found in directory: " << stopwordDir
              << std::endl;
  }

  //加载
  for (const auto &file : files) {
    std::ifstream ifs(file);
    if (!ifs.is_open()) {
      std::cerr << "Failed to open stopword file: " << file << std::endl;
      continue;
    }
    std::string word;
    while (ifs >> word) {
      stopWords_.insert(word);
    }
  }
}

bool PageProcessor::extract_documents(const std::string &dir) {
  using namespace tinyxml2;
  //获取目录下所有文件名称
  std::vector<std::string> files = DirectoryScanner::scan(dir);
  if (files.empty()) {
    std::cerr << "No XML files found in directory: " << dir << std::endl;
  }

  long uuid = 1;             //从1开始自增
  std::regex reg("<[^>]+>"); // 正则表达式：匹配HTML/XML标签，用来去除标签

  for (const auto &file : files) {
    XMLDocument doc;
    //加载XML文件
    XMLError loadResult = doc.LoadFile(file.c_str());
    if (loadResult != XML_SUCCESS) {
      std::cerr << "Failed to load XML file: " << file
                << ", error: " << doc.ErrorStr() << std::endl;
      continue;
    }

    XMLElement *root = doc.RootElement();
    if (root == nullptr) {
      std::cerr << "XML file has no root element: " << file << std::endl;
      continue;
    }

    //获取根节点下的第一个<channel>节点
    XMLElement *channel = nullptr;
    if (std::string(root->Name()) == "channel") {
      channel = root;
    } else {
      channel = root->FirstChildElement("channel");
    }

    if (channel == nullptr) {
      std::cerr << "XML file has no <channel> element: " << file << std::endl;
      continue;
    }

    while (channel) {
      XMLElement *item = channel->FirstChildElement("item");
      while (item) {
        //遍历每个<item>（RSS中的一篇文章）
        Document x;
        x.id = uuid++;

        //提取<title>
        XMLElement *title = item->FirstChildElement("title");
        x.title = extract_clean_text(title, reg);

        //提取<link>
        XMLElement *link = item->FirstChildElement("link");
        x.link = extract_clean_text(link, reg);

        //提取<description>
        XMLElement *description = item->FirstChildElement("description");
        x.content = extract_clean_text(description, reg);

        //提取<content>(优先级更高，会覆盖description)
        XMLElement *content = item->FirstChildElement("content");
        std::string extractedContent = extract_clean_text(content, reg);
        if (!extractedContent.empty()) {
          x.content = extractedContent;
        }

        //只保留有正文内容的文档
        if (!x.content.empty()) {
          documents_.push_back(x);
        }
        //下一个item
        item = item->NextSiblingElement("item");
      }
      //下一个channel
      channel = channel->NextSiblingElement("channel");
    }
  }

  return true;
}

//用simhash去重
void PageProcessor::deduplicate_documents() {
  //存储去重后的文档
  std::vector<Document> deduplicatedDocuments;
  //存储每个文档的simhash值
  std::vector<uint64_t> simhashes;

  for (size_t i = 0; i < documents_.size(); i++) {
    uint64_t h;
    const std::string &text = documents_[i].content;

    //动态计算topN。文本越长，取的词越多(范围在5~200之间)
    int topN = std::max(5, std::min(200, (int)text.size() / 120));

    //计算当前文档的simhash
    hasher_.make(text, topN, h);

    //如果当前文档的simhash是新值，保留
    if (!hash_exists(simhashes, h)) {
      deduplicatedDocuments.push_back(documents_[i]);
      simhashes.push_back(h);
    }
  }

  //用去重后的文档替换原文档
  documents_ = std::move(deduplicatedDocuments);
}

//构建网页库和偏移库
bool PageProcessor::build_pages_and_offsets(const std::string &pages,
                                            const std::string &offsets) {
  //存储完整的XML格式文档
  std::ofstream ofsPage(pages);
  //存储每个文档的偏移信息
  std::ofstream ofsOffset(offsets);
  if (!ofsPage.is_open()) {
    std::cerr << "Failed to open pages output file: " << pages << std::endl;
    return false;
  }
  if (!ofsOffset.is_open()) {
    std::cerr << "Failed to open offsets output file: " << offsets
              << std::endl;
    return false;
  }

  for (const auto &doc : documents_) {
    //获取当前文档在pages文件中的起始位置
    auto start = ofsPage.tellp();

    //将文档以XML格式写入网页库
    ofsPage << "<doc>\n"
            << " <id>" << doc.id << "</id>\n"
            << " <link>" << doc.link << "</link>\n"
            << " <title>" << doc.title << "</title>\n"
            << " <content>" << doc.content << "</content>\n"
            << " </doc>\n";

    //获取当前文档的结束位置
    auto end = ofsPage.tellp();
    //写入：文档ID 起始位置 文档长度(偏移信息)
    ofsOffset << doc.id << " " << start << " " << end - start << "\n";
  }

  return true;
}

//构建倒排索引
bool PageProcessor::build_inverted_index(const std::string &path) {

  //总文档数
  int N = static_cast<int>(documents_.size());

  //正排索引:<文档id,<关键词,词频(TF)>>
  std::map<int, std::map<std::string, int>> index;

  //文档频率:<关键词,DF(包含该词的文档数)>
  std::map<std::string, int> docFreq;

  //统计TF和DF
  for (const auto &doc : documents_) {
    //分词
    std::vector<std::string> words;
    tokenizer_.Cut(doc.content, words);

    //过滤，只保留纯中文且非停用词的词语
    words.erase(std::remove_if(words.begin(), words.end(),
                               [this](const std::string &word) {
                                 return !chinese_word(word) ||
                                        (this->stopWords_.count(word) > 0);
                               }),
                words.end());

    //统计当前文档中每个词出现的次数(TF)
    std::map<std::string, int> wordFreq;
    for (const auto &word : words) {
      wordFreq[word]++;
    }

    //更新DF，每个词只要在当前文档中出现过，DF就+1
    std::set<std::string> wordSet(words.begin(), words.end());
    for (const auto &word : wordSet) {
      docFreq[word]++;
    }

    //保存当前文档的正排索引
    index[doc.id] = std::move(wordFreq);
  }

  //计算TF-IDF并归一化，生成倒排索引
  for (const auto &[id, wordFreq] : index) {
    // 存储当前文档所有词的原始TF-IDF权重
    std::vector<std::pair<std::string, double>> weights;

    for (const auto &[word, TF] : wordFreq) {
      double DF = docFreq[word];
      double IDF = std::log2(N / (DF + 1)); // 加1平滑，避免除以0
      double w = TF * IDF;             //原始TF-IDF权重
      weights.push_back(std::make_pair(word, w));
    }

    //计算归一化因子(L2范数)
    double square = 0.0;
    for (const auto &[_, w] : weights) {
      square += w * w;
    }
    double squareRoot = std::sqrt(square);
    if (squareRoot == 0.0) {
      continue;
    }

    //归一化后写入倒排索引
    for (const auto &[word, w] : weights) {
      double w1 = w / squareRoot; //归一化后的权重
      invertedIndex_[word][id] = w1;
    }
  }

  //将倒排索引写入文件
  std::ofstream ofs(path);
  if (!ofs.is_open()) {
    std::cerr << "Failed to open inverted index output file: " << path
              << std::endl;
    return false;
  }
  for (const auto &[word, m] : invertedIndex_) {
    ofs << word << " ";
    for (const auto &[id, weight] : m) {
      ofs << id << " " << weight << " ";
    }
    ofs << "\n";
  }

  return true;
}
