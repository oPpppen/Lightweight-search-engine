//#include <assert.h>
#include <cppjieba/Jieba.hpp>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <utfcpp/utf8.h>
#include <vector>

#include "DirectoryScanner.h"
#include "KeywordProcessor.h"

//根据Unicode码点，判断是否为汉字
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

//判断一个字符串是否全由中文组成
bool chinese_string(const std::string &str) {
  auto it = utf8::iterator<std::string::const_iterator>(str.begin(),
                                                        str.begin(), str.end());
  auto end = utf8::iterator<std::string::const_iterator>(str.end(), str.begin(),
                                                         str.end());

  while (it != end) {
    if (!chinese_character(*it))
      return false;
    it++;
  }

  return true;
}

//加载中英文停用词
KeyWordProcessor::KeyWordProcessor() {
  {
    //加载中文停用词
    std::ifstream ifsCh("../stopwords/stopwords_cn.txt");
    std::string word;
    while (ifsCh >> word) {
      chStopWords_.insert(word);
    }
    ifsCh.close();

    //加载英文停用词
    std::ifstream ifsEn("../stopwords/stopwords_en.txt");
    while (ifsEn >> word) {
      enStopWords_.insert(word);
    }
    ifsEn.close();
  }
}

void KeyWordProcessor::create_cn_dict(const std::string &chDir,
                                      const std::string &chDict) {

  //获取目录下的所有文件名
  std::vector<std::string> files = DirectoryScanner::scan(chDir);
  //词->词频
  std::map<std::string, int> dict;

  for (const auto &file : files) {

    std::ifstream ifs(file);
    std::string line;
    while (getline(ifs, line)) {
      std::vector<std::string> words;
      //结巴分词
      tokenizer_.Cut(line, words);

      //只保留中文且非停用词词语
      words.erase(remove_if(words.begin(), words.end(),
                            [this](const std::string &word) {
                              return !chinese_string(word) ||
                                     chStopWords_.count(word) > 0;
                            }),
                  words.end());
      //统计词频
      for (const auto &word : words) {
        dict[word]++;
      }
    }
    ifs.close();
  }

  //写入词典文件:词 词频
  std::ofstream ofs(chDict);
  for (const auto &[key, val] : dict) {
    ofs << key << " " << val << "\n";
  }
  ofs.close();
}
void KeyWordProcessor::build_cn_index(const std::string &chDict,
                                      const std::string &chIndex) {
  std::ifstream ifs(chDict);
  std::string content;
  int line = 1;

  //构建索引表(汉字->行号)
  std::map<std::string, std::set<int>> result;
  while (getline(ifs, content)) {
    //提取词
    std::string word = content.substr(0, content.find(' '));

    //将词拆为单个字
    const char *it = word.c_str();
    const char *end = word.c_str() + word.size();
    while (it != end) {
      const char *start = it;
      utf8::next(it, end);
      std::string character(start, it);
      result[character].insert(line);
    }
    line++;
  }
  ifs.close();

  //写入索引文件:汉字 行号1 行号2...
  std::ofstream ofs(chIndex);
  for (const auto &[character, lineNumbers] : result) {
    ofs << character << " ";
    for (auto lineNumber : lineNumbers) {
      ofs << lineNumber << " ";
    }
    ofs << "\n";
  }
  ofs.close();
}

void KeyWordProcessor::create_en_dict(const std::string &enDir,
                                      const std::string &enDict) {

  std::map<std::string, int> dict;
  std::vector<std::string> files = DirectoryScanner::scan(enDir);

  for (const auto &file : files) {
    std::ifstream ifs(file);
    std::string line;
    while (getline(ifs, line)) {
      // 大写字母转小写并且将非字母字符替换为空格
      for (auto &c : line) {
        if (!isalpha(c)) {
          c = ' ';
        } else {
          c = tolower(c);
        }
      }

      std::stringstream ss(line);
      std::string word;
      while (ss >> word) { // 按空白字符分割单词
        if (enStopWords_.find(word) != enStopWords_.end()) {
          continue; // 跳过停用词
        }
        dict[word]++;
      }
    }
    ifs.close();
  }

  // 写入英文词典：单词 词频
  std::ofstream ofs(enDict);
  for (const auto &[key, val] : dict) {
    ofs << key << " " << val << "\n";
  }
  ofs.close();
}

void KeyWordProcessor::build_en_index(const std::string &enDict,
                                      const std::string &enIndex) {

  std::ifstream ifs(enDict);
  std::string content;
  int line = 1;
  std::map<std::string, std::set<int>> result; // 字母 → 包含该字母的单词所在行号

  while (getline(ifs, content)) {
    std::string word = content.substr(0, content.find(' ')); // 提取单词

    // 遍历单词中的每个字母
    for (const auto &c : word) {
      std::string character{c};
      result[character].insert(line);
    }
    line++;
  }
  ifs.close();

  // 写入索引文件：字母 行号1 行号2 ...
  std::ofstream ofs(enIndex);
  for (const auto &[character, lines] : result) {
    ofs << character << " ";
    for (auto line : lines) {
      ofs << line << " ";
    }
    ofs << "\n";
  }
  ofs.close();
}
