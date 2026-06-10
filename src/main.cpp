#include "compiler.h"
#include "comparator.h"
#include "runner.h"

#include <filesystem>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    // 支持两种命令格式：
    // 1. 单测试点模式：
    //    ./minijudge source.cpp input.txt answer.txt
    // 2. 多测试点模式：
    //    ./minijudge source.cpp problem_dir
    if (argc != 3 && argc != 4) {
        cerr << "Usage 1: ./minijudge <source.cpp> <input.txt> <answer.txt>" << endl;
        cerr << "Usage 2: ./minijudge <source.cpp> <problem_dir>" << endl;
        return 1;
    }

    string sourceFile = argv[1];

    bool multiCaseMode = (argc == 3);

    string inputFile;
    string answerFile;
    string problemDir;

    if (multiCaseMode) {
        problemDir = argv[2];
    } else {
        inputFile = argv[2];
        answerFile = argv[3];
    }

    string tmpDir = "judge_tmp";

    // 创建临时目录，用来存放编译产物和输出文件
    filesystem::create_directories(tmpDir);

    string exeFile = tmpDir + "/main";
    string outputFile = tmpDir + "/output.txt";
    string errorFile = tmpDir + "/error.txt";
    string compileErrorFile = tmpDir + "/compile_error.txt";

    int timeLimitMs = 2000;

    // 第一步：编译用户代码
    bool compileOk = compileCode(sourceFile, exeFile, compileErrorFile);

    if (!compileOk) {
        cout << "Result: CE" << endl;
        cout << "Compile Error saved to " << compileErrorFile << endl;
        return 0;
    }

    cout << "Compile: OK" << endl;

    if (!multiCaseMode) {
        // 单测试点模式
        int timeUsedMs = 0;

        RunResult runResult = runProgram(
            exeFile,
            inputFile,
            outputFile,
            errorFile,
            timeLimitMs,
            timeUsedMs
        );

        if (runResult == RunResult::TLE) {
            cout << "Result: TLE" << endl;
            cout << "Time: " << timeUsedMs << " ms" << endl;
            return 0;
        }

        if (runResult == RunResult::RE) {
            cout << "Result: RE" << endl;
            cout << "Time: " << timeUsedMs << " ms" << endl;
            return 0;
        }

        bool accepted = compareFiles(outputFile, answerFile);

        if (accepted) {
            cout << "Result: AC" << endl;
        } else {
            cout << "Result: WA" << endl;
        }

        cout << "Time: " << timeUsedMs << " ms" << endl;
        return 0;
    }

    // 多测试点模式
    int caseId = 1;
    int totalTimeMs = 0;
    bool hasCase = false;
    string finalResult = "AC";

    while (true) {
        string caseInputFile = problemDir + "/" + to_string(caseId) + ".in";
        string caseAnswerFile = problemDir + "/" + to_string(caseId) + ".out";

        if (!filesystem::exists(caseInputFile) || !filesystem::exists(caseAnswerFile)) {
            break;
        }

        hasCase = true;

        int timeUsedMs = 0;

        RunResult runResult = runProgram(
            exeFile,
            caseInputFile,
            outputFile,
            errorFile,
            timeLimitMs,
            timeUsedMs
        );

        totalTimeMs += timeUsedMs;

        cout << "Test " << caseId << ": ";

        if (runResult == RunResult::TLE) {
            cout << "TLE";
            finalResult = "TLE";
            cout << " (" << timeUsedMs << " ms)" << endl;
            break;
        }

        if (runResult == RunResult::RE) {
            cout << "RE";
            finalResult = "RE";
            cout << " (" << timeUsedMs << " ms)" << endl;
            break;
        }

        bool accepted = compareFiles(outputFile, caseAnswerFile);

        if (accepted) {
            cout << "AC";
        } else {
            cout << "WA";
            finalResult = "WA";
            cout << " (" << timeUsedMs << " ms)" << endl;
            break;
        }

        cout << " (" << timeUsedMs << " ms)" << endl;

        caseId++;
    }

    if (!hasCase) {
        cout << "Result: No test cases found" << endl;
        return 0;
    }

    cout << "Final: " << finalResult << endl;
    cout << "Total Time: " << totalTimeMs << " ms" << endl;

    return 0;
}
