#pragma once
#include <set>
#include <string>
#include <vector>

#include "cppjieba/Jieba.hpp"    // 中文分词
#include "simhash/Simhasher.hpp" //文档去重

//网页文档处理类。
//负责从XML文件中提取网页文档、去重。构建网页库和倒排索引
class PageProcessor {
public:
  //构造函数，加载停用词表
  PageProcessor();

  void process(const std::string &dir) {
    // 1.从XML中提取文档
    extract_documents(dir);
    // 2.去重
    deduplicate_documents();
    // 3.构建网页库和偏移库
    build_pages_and_offsets("../pages.dat", "../offsets.dat");
    // 4.构建倒排索引
    build_inverted_index("../inverted_index.dat");
  }

private:
  //从XML目录中提取文档
  void extract_documents(const std::string &dir);

  //用simhash对文档去重
  void deduplicate_documents();

  //构建网页库和偏移库
  void build_pages_and_offsets(const std::string &pages,
                               const std::string &offsets);

  //构建倒排索引
  void build_inverted_index(const std::string &filename);

private:
  //文档结构体，表示一个网页文档
  struct Document {

    int id;              //唯一ID
    std::string link;    //网页链接
    std::string title;   //网页标题
    std::string content; //网页内容
  };

  cppjieba::Jieba tokenizer_;       //中文分词器
  simhash::Simhasher hasher_;       //文档去重器
  std::set<std::string> stopWords_; //停用词集合
  std::vector<Document> documents_; //文档
  //倒排索引：词-><文档ID,权重>
  std::map<std::string, std::map<int, double>> invertedIndex_;
};
