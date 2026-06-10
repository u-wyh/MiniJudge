#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>
#include <csignal>
#include <thread>
#include <filesystem>

using namespace std;

enum class RunResult {
    OK,
    RE,
    TLE
};

// 编译用户提交的 C++ 源码
bool compileCode(const string& sourceFile, const string& exeFile, const string& compileErrorFile) {
    pid_t pid = fork();

    if (pid < 0) {
        cerr << "fork failed" << endl;
        return false;
    }

    if (pid == 0) {
        // 把 g++ 的错误信息重定向到 compile_error.txt
        int errFd = open(compileErrorFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (errFd < 0) {
            cerr << "open compile error file failed" << endl;
            _exit(1);
        }

        dup2(errFd, STDERR_FILENO);
        close(errFd);

        // 等价于：g++ sourceFile -O2 -std=c++17 -o exeFile
        execlp(
            "g++",
            "g++",
            sourceFile.c_str(),
            "-O2",
            "-std=c++17",
            "-o",
            exeFile.c_str(),
            nullptr
        );

        cerr << "exec g++ failed" << endl;
        _exit(1);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

// 运行用户程序，并限制运行时间
RunResult runProgram(const string& exeFile,
                     const string& inputFile,
                     const string& outputFile,
                     const string& errorFile,
                     int timeLimitMs) {
    pid_t pid = fork();

    if (pid < 0) {
        cerr << "fork failed" << endl;
        return RunResult::RE;
    }

    if (pid == 0) {
        int inFd = open(inputFile.c_str(), O_RDONLY);
        int outFd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        int errFd = open(errorFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (inFd < 0 || outFd < 0 || errFd < 0) {
            cerr << "open file failed" << endl;
            _exit(1);
        }

        // stdin <- inputFile
        dup2(inFd, STDIN_FILENO);

        // stdout -> outputFile
        dup2(outFd, STDOUT_FILENO);

        // stderr -> errorFile
        dup2(errFd, STDERR_FILENO);

        close(inFd);
        close(outFd);
        close(errFd);

        // 执行用户程序
        execl(exeFile.c_str(), exeFile.c_str(), nullptr);

        cerr << "exec user program failed" << endl;
        _exit(1);
    }

    int status = 0;

    auto start = chrono::steady_clock::now();

    while (true) {
        // WNOHANG 表示非阻塞等待
        // 如果子进程还没结束，waitpid 立刻返回 0
        pid_t ret = waitpid(pid, &status, WNOHANG);

        if (ret == pid) {
            // 子进程已经结束
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                return RunResult::OK;
            }

            return RunResult::RE;
        }

        if (ret < 0) {
            return RunResult::RE;
        }

        auto now = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - start).count();

        if (elapsed > timeLimitMs) {
            // 超时后杀死用户程序
            kill(pid, SIGKILL);

            // 回收子进程，避免僵尸进程
            waitpid(pid, &status, 0);

            return RunResult::TLE;
        }

        // 稍微睡一下，避免父进程一直空转占 CPU
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

// 读取整个文件内容
string readFile(const string& fileName) {
    ifstream fin(fileName);
    stringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}

// 比较用户输出和标准答案
bool compareFiles(const string& outputFile, const string& answerFile) {
    string output = readFile(outputFile);
    string answer = readFile(answerFile);

    return output == answer;
}

int main(int argc, char* argv[]) {
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

    bool compileOk = compileCode(sourceFile, exeFile, compileErrorFile);

    if (!compileOk) {
        cout << "Result: CE" << endl;
        cout << "Compile Error saved to " << compileErrorFile << endl;
        return 0;
    }

    cout << "Compile: OK" << endl;

    RunResult runResult = runProgram(
        exeFile,
        inputFile,
        outputFile,
        errorFile,
        timeLimitMs
    );

    if (runResult == RunResult::TLE) {
        cout << "Result: TLE" << endl;
        return 0;
    }

    if (runResult == RunResult::RE) {
        cout << "Result: RE" << endl;
        return 0;
    }

    bool accepted = compareFiles(outputFile, answerFile);

    if (accepted) {
        cout << "Result: AC" << endl;
    } else {
        cout << "Result: WA" << endl;
    }

    return 0;
}
