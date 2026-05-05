#pragma once
#include <cppjieba/Jieba.hpp>
#include <set>
#include <string>

class KeyWordProcessor {
public:
  //构造函数，加载中英文停用词表
  KeyWordProcessor();

  //根据目录，生成词典库和索引库
  void process(const std::string &chDir, const std::string &enDir) {
    //生成中文词典(词->词频)
    create_cn_dict(chDir, "../ch_dict.dat");
    //构建中文索引(汉字->行号)
    build_cn_index("../ch_dict.dat", "../ch_index.dat");

    //生成英文词典(词->词频)
    create_en_dict(enDir, "../en_dict.dat");
    //构建英文索引(字母->行号)
    build_en_index("../en_dict.dat", "../en_index.dat");
  }

private:
  //中文词典库和索引表
  void create_cn_dict(const std::string &chDir, const std::string &chDict);
  void build_cn_index(const std::string &chDict, const std::string &chIndex);

  //英文词典库和索引表
  void create_en_dict(const std::string &enDir, const std::string &enDict);
  void build_en_index(const std::string &enDict, const std::string &enIndex);

private:
  //分词器对象
  cppjieba::Jieba tokenizer_;
  //中文停用词set
  std::set<std::string> chStopWords_;
  //英文停用词set
  std::set<std::string> enStopWords_;
};