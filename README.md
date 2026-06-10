# MiniJudge

MiniJudge 是一个用 C++ 实现的轻量级在线判题核心 / 代码运行沙盒。

## Phase 1 目标

第一阶段实现一个命令行工具：

    ./minijudge examples/ac.cpp tests/input.txt tests/answer.txt

支持判题结果：

- AC: Accepted
- WA: Wrong Answer
- CE: Compile Error
- RE: Runtime Error
- TLE: Time Limit Exceeded

## 技术点

- C++17
- Linux 系统调用
- fork / exec / waitpid
- dup2 标准输入输出重定向
- kill 超时控制
