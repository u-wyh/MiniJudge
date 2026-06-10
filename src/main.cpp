#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

// 编译用户提交的 C++ 源码
// sourceFile: 用户代码路径，例如 examples/ac.cpp
// exeFile: 编译后生成的可执行文件路径，例如 ./judge_tmp_main
bool compileCode(const string& sourceFile, const string& exeFile) {
    // fork 创建子进程
    // 子进程负责执行 g++ 编译
    // 父进程负责等待编译结束
    pid_t pid = fork();

    if (pid < 0) {
        cerr << "fork failed" << endl;
        return false;
    }

    if (pid == 0) {
        // 子进程执行 g++
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

        // 如果 execlp 成功，子进程会被 g++ 替换，不会继续执行到这里
        // 能走到这里，说明 exec 失败
        cerr << "exec g++ failed" << endl;
        _exit(1);
    }

    int status = 0;

    // 父进程等待编译进程结束
    waitpid(pid, &status, 0);

    // WIFEXITED(status): 子进程是否正常退出
    // WEXITSTATUS(status) == 0: g++ 返回值是否为 0
    // g++ 返回 0 表示编译成功，非 0 表示编译错误
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

// 运行用户程序，并完成标准输入、标准输出、标准错误重定向
// exeFile: 要运行的用户程序
// inputFile: 标准输入文件
// outputFile: 用户程序输出文件
// errorFile: 用户程序错误输出文件
bool runProgram(const string& exeFile, const string& inputFile, const string& outputFile, const string& errorFile) {
    // fork 创建子进程
    // 子进程负责运行用户程序
    // 父进程负责等待用户程序结束
    pid_t pid = fork();

    if (pid < 0) {
        cerr << "fork failed" << endl;
        return false;
    }

    if (pid == 0) {
        // 打开输入文件
        int inFd = open(inputFile.c_str(), O_RDONLY);

        // 打开输出文件
        // O_CREAT: 不存在就创建
        // O_TRUNC: 如果已经存在，就清空原内容
        int outFd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

        // 打开错误输出文件
        int errFd = open(errorFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (inFd < 0 || outFd < 0 || errFd < 0) {
            cerr << "open file failed" << endl;
            _exit(1);
        }

        // 把 inputFile 重定向到标准输入 stdin
        // 等价于 shell 中的：
        // ./program < input.txt
        dup2(inFd, STDIN_FILENO);

        // 把 outputFile 重定向到标准输出 stdout
        // 等价于 shell 中的：
        // ./program > output.txt
        dup2(outFd, STDOUT_FILENO);

        // 把 errorFile 重定向到标准错误 stderr
        // 等价于 shell 中的：
        // ./program 2> error.txt
        dup2(errFd, STDERR_FILENO);

        // dup2 之后，原来的 fd 已经可以关闭
        close(inFd);
        close(outFd);
        close(errFd);

        // 执行用户程序
        // 注意：exeFile 这里一般是 ./judge_tmp_main
        execl(exeFile.c_str(), exeFile.c_str(), nullptr);

        // 如果 execl 成功，当前子进程会被用户程序替换，不会继续执行到这里
        // 能走到这里，说明 exec 失败
        cerr << "exec user program failed" << endl;
        _exit(1);
    }

    int status = 0;

    // 父进程等待用户程序运行结束
    waitpid(pid, &status, 0);

    // 正常退出并且返回值为 0，认为运行成功
    // 如果段错误、异常退出、返回非 0，认为 RE
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

// 读取整个文件内容，返回 string
string readFile(const string& fileName) {
    ifstream fin(fileName);
    stringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}

// 比较用户输出文件和标准答案文件
// 第一版先做最简单的完全一致比较
bool compareFiles(const string& outputFile, const string& answerFile) {
    string output = readFile(outputFile);
    string answer = readFile(answerFile);

    return output == answer;
}

int main(int argc, char* argv[]) {
    // 命令格式：
    // ./minijudge source.cpp input.txt answer.txt
    if (argc != 4) {
        cerr << "Usage: ./minijudge <source.cpp> <input.txt> <answer.txt>" << endl;
        return 1;
    }

    // argv[1]: 用户提交的 C++ 源码
    // argv[2]: 测试输入文件
    // argv[3]: 标准答案文件
    string sourceFile = argv[1];
    string inputFile = argv[2];
    string answerFile = argv[3];

    // 临时生成的用户程序
    string exeFile = "./judge_tmp_main";

    // 用户程序运行后的输出文件
    string outputFile = "output.txt";

    // 用户程序运行后的错误输出文件
    string errorFile = "error.txt";

    // 第一步：编译用户代码
    bool compileOk = compileCode(sourceFile, exeFile);

    if (!compileOk) {
        cout << "Result: CE" << endl;
        return 0;
    }

    cout << "Compile: OK" << endl;

    // 第二步：运行用户程序
    bool runOk = runProgram(exeFile, inputFile, outputFile, errorFile);

    if (!runOk) {
        cout << "Result: RE" << endl;
        return 0;
    }

    // 第三步：比较用户输出和标准答案
    bool accepted = compareFiles(outputFile, answerFile);

    if (accepted) {
        cout << "Result: AC" << endl;
    } else {
        cout << "Result: WA" << endl;
    }

    return 0;
}
