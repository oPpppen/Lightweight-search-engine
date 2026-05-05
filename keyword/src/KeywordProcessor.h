#pragma once
#include <cppjieba/Jieba.hpp>
#include <set>
#include <string>

class KeyWordProcessor {
public:
  //构造函数，加载中英文停用词表
  KeyWordProcessor(const std::string &jiebaDictDir,
                   const std::string &stopwordDir,
                   const std::string &outputDir);

  //根据目录，生成词典库和索引库
  bool process(const std::string &chDir, const std::string &enDir) {
    //生成中文词典(词->词频)
    if (!create_cn_dict(chDir, outputDir_ + "/ch_dict.dat")) {
      return false;
    }
    //构建中文索引(汉字->行号)
    if (!build_cn_index(outputDir_ + "/ch_dict.dat", outputDir_ + "/ch_index.dat")) {
      return false;
    }

    //生成英文词典(词->词频)
    if (!create_en_dict(enDir, outputDir_ + "/en_dict.dat")) {
      return false;
    }
    //构建英文索引(字母->行号)
    if (!build_en_index(outputDir_ + "/en_dict.dat", outputDir_ + "/en_index.dat")) {
      return false;
    }

    return true;
  }

private:
  //中文词典库和索引表
  bool create_cn_dict(const std::string &chDir, const std::string &chDict);
  bool build_cn_index(const std::string &chDict, const std::string &chIndex);

  //英文词典库和索引表
  bool create_en_dict(const std::string &enDir, const std::string &enDict);
  bool build_en_index(const std::string &enDict, const std::string &enIndex);

private:
  //分词器对象
  cppjieba::Jieba tokenizer_;
  //中文停用词set
  std::set<std::string> chStopWords_;
  //英文停用词set
  std::set<std::string> enStopWords_;
  //统一输出目录
  std::string outputDir_;
};
