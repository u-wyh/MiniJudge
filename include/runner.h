#ifndef RUNNER_H
#define RUNNER_H

#include <string>
#include "judge_result.h"

// 运行用户程序，并完成输入输出重定向和超时控制
// exeFile: 用户程序路径
// inputFile: 输入文件路径
// outputFile: 输出文件路径
// errorFile: 错误输出文件路径
// timeLimitMs: 时间限制，单位毫秒
// timeUsedMs: 实际运行时间，单位毫秒
RunResult runProgram(const std::string& exeFile,
                     const std::string& inputFile,
                     const std::string& outputFile,
                     const std::string& errorFile,
                     int timeLimitMs,
                     int& timeUsedMs);

#endif
