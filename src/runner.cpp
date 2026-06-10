#include "runner.h"

#include <chrono>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <sys/resource.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

using namespace std;

RunResult runProgram(const string& exeFile,
                     const string& inputFile,
                     const string& outputFile,
                     const string& errorFile,
                     int timeLimitMs,
                     int& timeUsedMs) {
    pid_t pid = fork();

    if (pid < 0) {
        cerr << "fork failed" << endl;
        return RunResult::RE;
    }

    if (pid == 0) {
        // 禁止用户程序崩溃时生成 core dump 文件
        struct rlimit coreLimit;
        coreLimit.rlim_cur = 0;
        coreLimit.rlim_max = 0;
        setrlimit(RLIMIT_CORE, &coreLimit);

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

        // execl 成功后不会继续执行到这里
        cerr << "exec user program failed" << endl;
        _exit(1);
    }

    int status = 0;
    auto start = chrono::steady_clock::now();
    timeUsedMs = 0;

    while (true) {
        // WNOHANG 表示非阻塞等待
        pid_t ret = waitpid(pid, &status, WNOHANG);

        if (ret == pid) {
            auto now = chrono::steady_clock::now();
            timeUsedMs = chrono::duration_cast<chrono::milliseconds>(now - start).count();

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
            timeUsedMs = elapsed;

            // 超时后杀死用户程序
            kill(pid, SIGKILL);

            // 回收子进程，避免僵尸进程
            waitpid(pid, &status, 0);

            return RunResult::TLE;
        }

        // 避免父进程一直空转占 CPU
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}
