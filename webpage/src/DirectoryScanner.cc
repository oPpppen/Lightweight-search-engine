#include "DirectoryScanner.h"
#include <dirent.h>
#include <string.h>

std::vector<std::string> DirectoryScanner::scan(const std::string &dir) {
  std::vector<std::string> result;

  //打开目录流
  DIR *dirp = opendir(dir.c_str());
  if (!dirp) {
    return result;
  }

  //此处，成功打开目录流

  //遍历目录项
  struct dirent *curr;
  while ((curr = readdir(dirp)) != NULL) {
    //忽略.（当前目录）和..(上一级目录)
    if (strcmp(".", curr->d_name) == 0 || strcmp("..", curr->d_name) == 0)
      continue;

    //普通文件
    if (curr->d_type == DT_REG)
      result.emplace_back(dir + "/" + curr->d_name);
    //目录文件
    else if (curr->d_type == DT_DIR) {
      //递归遍历该子目录
      auto subFiles = scan(dir + "/" + curr->d_name);
      result.insert(result.end(), subFiles.begin(), subFiles.end());
    }
  }

  //关闭目录流
  closedir(dirp);
  return result;
}