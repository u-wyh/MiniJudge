#pragma once

#include "judge_result.h"
#include "judge_config.h"

#include <string>

RunResult runProgram(const std::string& exeFile,
                     const std::string& inputFile,
                     const std::string& outputFile,
                     const std::string& errorFile,
                     const JudgeConfig& config,
                     int& timeUsedMs);
