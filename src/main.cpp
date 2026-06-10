#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

bool compileCode(const string& sourceFile, const string& exeFile) {
    pid_t pid = fork();

    if (pid < 0) {
        cerr << "fork failed" << endl;
        return false;
    }

    if (pid == 0) {
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

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true;
    }

    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: ./minijudge <source.cpp> <input.txt> <answer.txt>" << endl;
        return 1;
    }

    string sourceFile = argv[1];
    string inputFile = argv[2];
    string answerFile = argv[3];

    string exeFile = "judge_tmp_main";

    cout << "Source: " << sourceFile << endl;
    cout << "Input: " << inputFile << endl;
    cout << "Answer: " << answerFile << endl;

    bool ok = compileCode(sourceFile, exeFile);

    if (!ok) {
        cout << "Result: CE" << endl;
        return 0;
    }

    cout << "Compile: OK" << endl;

    return 0;
}
