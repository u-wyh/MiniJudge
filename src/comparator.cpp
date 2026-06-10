#include "comparator.h"

#include <fstream>
#include <sstream>
#include <string>
using namespace std;

static string lastCompareMessage;

static string charToString(char c) {
    if (c == '\n') return "\\n";
    if (c == '\t') return "\\t";
    if (c == '\r') return "\\r";
    if (c == ' ') return "space";
    return string(1, c);
}

static bool isBlankChar(char c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static bool readFile(const string& path, string& content) {
    ifstream file(path, ios::binary);
    if (!file.is_open()) {
        return false;
    }

    content.assign(
        istreambuf_iterator<char>(file),
        istreambuf_iterator<char>()
    );

    return true;
}

static void removeTrailingBlank(string& s) {
    while (!s.empty() && isBlankChar(s.back())) {
        s.pop_back();
    }
}

static void setCompareFailedMessage(
    int line,
    int column,
    const string& reason,
    const string& outputChar,
    const string& answerChar
) {
    ostringstream oss;

    oss << "Compare failed at line " << line << ", column " << column << '\n';

    if (!reason.empty()) {
        oss << reason << '\n';
    }

    if (!outputChar.empty()) {
        oss << "Output char: '" << outputChar << "'" << '\n';
    }

    if (!answerChar.empty()) {
        oss << "Answer char: '" << answerChar << "'" << '\n';
    }

    lastCompareMessage = oss.str();
}

string getLastCompareMessage() {
    return lastCompareMessage;
}

bool compareFiles(const string& outputFile, const string& answerFile) {
    lastCompareMessage.clear();

    string output;
    string answer;

    if (!readFile(outputFile, output) || !readFile(answerFile, answer)) {
        lastCompareMessage = "Compare Error: failed to open output or answer file\n";
        return false;
    }

    removeTrailingBlank(output);
    removeTrailingBlank(answer);

    int line = 1;
    int column = 1;

    size_t n = min(output.size(), answer.size());

    for (size_t i = 0; i < n; i++) {
        if (output[i] != answer[i]) {
            setCompareFailedMessage(
                line,
                column,
                "",
                charToString(output[i]),
                charToString(answer[i])
            );
            return false;
        }

        if (output[i] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }

    if (output.size() != answer.size()) {
        if (output.size() < answer.size()) {
            setCompareFailedMessage(
                line,
                column,
                "Output ended early",
                "",
                charToString(answer[n])
            );
        } else {
            setCompareFailedMessage(
                line,
                column,
                "Answer ended early",
                charToString(output[n]),
                ""
            );
        }

        return false;
    }

    return true;
}
