#pragma once
#include <string>
#include <vector>

//工具类，用于遍历目录dir，获取目录里面所有文件的路径
class DirectoryScanner {
  //因为是工具类，所以函数设置为静态。
  //直接通过   类名::调用
public:
  static std::vector<std::string> scan(const std::string &dir);

private:
  //构造函数删除
  DirectoryScanner() = delete;
};