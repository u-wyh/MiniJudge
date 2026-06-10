#ifndef JUDGE_RESULT_H
#define JUDGE_RESULT_H

// 用户程序运行阶段的结果
// OK  表示程序正常运行结束
// RE  表示运行时错误
// TLE 表示运行超时
enum class RunResult {
    OK,
    RE,
    TLE
};

#endif
