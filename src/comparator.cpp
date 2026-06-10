#include "comparator.h"

#include <fstream>
#include <sstream>
#include <string>

using namespace std;

// 读取整个文件内容
static string readFile(const string& fileName) {
    ifstream fin(fileName);
    stringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}

bool compareFiles(const string& outputFile,
                  const string& answerFile) {
    string output = readFile(outputFile);
    string answer = readFile(answerFile);

    return output == answer;
}
