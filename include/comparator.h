#ifndef COMPARATOR_H
#define COMPARATOR_H

#include <string>

// 比较用户输出文件和标准答案文件
// 当前版本使用完全一致比较
bool compareFiles(const std::string& outputFile,
                  const std::string& answerFile);

#endif
