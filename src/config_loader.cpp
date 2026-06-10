#include "config_loader.h"

#include <fstream>
#include <sstream>
#include <string>
using namespace std;

static string trim(const string& s) {
    size_t left = s.find_first_not_of(" \t\r\n");
    if (left == string::npos) {
        return "";
    }

    size_t right = s.find_last_not_of(" \t\r\n");
    return s.substr(left, right - left + 1);
}

bool loadJudgeConfig(const string& configFile, JudgeConfig& config) {
    ifstream fin(configFile);
    if (!fin.is_open()) {
        return false;
    }

    string line;

    while (getline(fin, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t pos = line.find('=');
        if (pos == string::npos) {
            continue;
        }

        string key = trim(line.substr(0, pos));
        string value = trim(line.substr(pos + 1));

        int v = stoi(value);

        if (key == "time_limit_ms") {
            config.timeLimitMs = v;
        } else if (key == "wait_interval_ms") {
            config.waitIntervalMs = v;
        } else if (key == "cpu_limit_seconds") {
            config.cpuLimitSeconds = v;
        } else if (key == "output_limit_mb") {
            config.outputLimitMb = v;
        } else if (key == "memory_limit_mb") {
            config.memoryLimitMb = v;
        } else if (key == "process_limit") {
            config.processLimit = v;
        }
    }

    return true;
}
