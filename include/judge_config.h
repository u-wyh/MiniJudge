#pragma once

struct JudgeConfig {
    int timeLimitMs = 2000;
    int waitIntervalMs = 1;

    int cpuLimitSeconds = 2;
    int outputLimitMb = 1;
    int memoryLimitMb = 128;
    int processLimit = 16;
};
