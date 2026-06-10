#include "compiler.h"

#include <fcntl.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

bool compileCode(const string& sourceFile,
                 const string& exeFile,
                 const string& compileErrorFile) {
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

        // 等价于：
        // g++ sourceFile -O2 -std=c++17 -o exeFile
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

        // execlp 成功后不会继续执行到这里
        cerr << "exec g++ failed" << endl;
        _exit(1);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
