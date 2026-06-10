#include "compiler.h"
#include "comparator.h"
#include "runner.h"

#include <filesystem>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    // 命令格式：
    // ./minijudge source.cpp input.txt answer.txt
    if (argc != 4) {
        cerr << "Usage: ./minijudge <source.cpp> <input.txt> <answer.txt>" << endl;
        return 1;
    }

    string sourceFile = argv[1];
    string inputFile = argv[2];
    string answerFile = argv[3];

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

    // 第二步：运行用户程序
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

    // 第三步：比较用户输出和标准答案
    bool accepted = compareFiles(outputFile, answerFile);

    if (accepted) {
        cout << "Result: AC" << endl;
    } else {
        cout << "Result: WA" << endl;
    }

    cout << "Time: " << timeUsedMs << " ms" << endl;

    return 0;
}
