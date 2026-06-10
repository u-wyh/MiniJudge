#ifndef COMPILER_H
#define COMPILER_H

#include <string>

// 编译用户提交的 C++ 源码
// sourceFile: 用户代码路径
// exeFile: 编译后生成的可执行文件路径
// compileErrorFile: 编译错误输出文件路径
bool compileCode(const std::string& sourceFile,
                 const std::string& exeFile,
                 const std::string& compileErrorFile);

#endif
