# MiniJudge

MiniJudge 是一个用 C++ 实现的轻量级在线判题核心 / 代码运行沙盒。

当前版本支持：

- Phase 1：单测试点命令行判题
- Phase 2：多测试点判题

## 1. 项目目标

MiniJudge 接收 C++ 源文件和测试数据，然后完成：

- 编译用户代码
- 运行用户程序
- 重定向标准输入、标准输出、标准错误
- 比较用户输出和标准答案
- 返回判题结果
- 支持单测试点和多测试点

## 2. 支持的判题结果

- AC: Accepted，答案正确
- WA: Wrong Answer，答案错误
- CE: Compile Error，编译错误
- RE: Runtime Error，运行时错误
- TLE: Time Limit Exceeded，运行超时

## 3. 编译 MiniJudge

    make

清理编译产物和临时目录：

    make clean

## 4. 单测试点模式

运行方式：

    ./minijudge examples/ac.cpp tests/input.txt tests/answer.txt

示例输出：

    Compile: OK
    Result: AC
    Time: 10 ms

## 5. 多测试点模式

运行方式：

    ./minijudge examples/ac.cpp problems/1001

题目目录格式：

    problems/
    └── 1001/
        ├── 1.in
        ├── 1.out
        ├── 2.in
        ├── 2.out
        ├── 3.in
        └── 3.out

示例输出：

    Compile: OK
    Test 1: AC (10 ms)
    Test 2: AC (10 ms)
    Test 3: AC (10 ms)
    Final: AC
    Total Time: 30 ms

多测试点模式下，遇到第一个 WA / RE / TLE 会立即停止。

## 6. 测试样例

单测试点数据：

- tests/input.txt
- tests/answer.txt

多测试点数据：

- problems/1001/1.in
- problems/1001/1.out
- problems/1001/2.in
- problems/1001/2.out
- problems/1001/3.in
- problems/1001/3.out

用户程序样例：

- examples/ac.cpp
- examples/wa.cpp
- examples/ce.cpp
- examples/re.cpp
- examples/tle.cpp

## 7. 回归测试

单测试点测试：

    ./minijudge examples/ac.cpp tests/input.txt tests/answer.txt
    ./minijudge examples/wa.cpp tests/input.txt tests/answer.txt
    ./minijudge examples/ce.cpp tests/input.txt tests/answer.txt
    ./minijudge examples/re.cpp tests/input.txt tests/answer.txt
    ./minijudge examples/tle.cpp tests/input.txt tests/answer.txt

多测试点测试：

    ./minijudge examples/ac.cpp problems/1001
    ./minijudge examples/wa.cpp problems/1001
    ./minijudge examples/re.cpp problems/1001
    ./minijudge examples/tle.cpp problems/1001

## 8. 当前代码结构

    MiniJudge/
    ├── include/
    │   ├── compiler.h
    │   ├── runner.h
    │   ├── comparator.h
    │   └── judge_result.h
    ├── src/
    │   ├── main.cpp
    │   ├── compiler.cpp
    │   ├── runner.cpp
    │   └── comparator.cpp
    ├── examples/
    ├── tests/
    ├── problems/
    ├── Makefile
    └── README.md

## 9. 当前技术点

- C++17
- fork 创建子进程
- execl / execlp 执行编译器和用户程序
- waitpid 等待子进程
- dup2 实现 stdin / stdout / stderr 重定向
- kill 实现超时终止
- setrlimit 禁用 core dump
- filesystem 创建临时目录和检测测试点
- chrono 统计运行时间

## 10. 临时文件

运行过程中会生成 judge_tmp 目录：

    judge_tmp/
    ├── main
    ├── output.txt
    ├── error.txt
    └── compile_error.txt

该目录不会提交到 Git。

## 11. 当前阶段限制

当前版本还不是安全沙盒。

目前尚未实现：

- Docker 隔离
- 内存限制
- 输出大小限制
- HTTP API
- Web 页面

当前版本只适合运行自己写的测试代码，不适合公开运行陌生人的代码。
