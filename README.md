# MiniJudge

MiniJudge 是一个用 C++ 实现的轻量级在线判题核心 / 代码运行沙盒。

当前版本是 Phase 1：单测试点命令行判题器。

## 1. 项目目标

MiniJudge 接收一个 C++ 源文件、一个输入文件和一个标准答案文件，然后完成：

- 编译用户代码
- 运行用户程序
- 重定向标准输入、标准输出、标准错误
- 比较用户输出和标准答案
- 返回判题结果

运行方式：

    ./minijudge examples/ac.cpp tests/input.txt tests/answer.txt

## 2. 当前支持的判题结果

- AC: Accepted，答案正确
- WA: Wrong Answer，答案错误
- CE: Compile Error，编译错误
- RE: Runtime Error，运行时错误
- TLE: Time Limit Exceeded，运行超时

## 3. 编译 MiniJudge

    make

清理编译产物：

    make clean

## 4. 测试样例

测试数据：

- tests/input.txt
- tests/answer.txt

用户程序样例：

- examples/ac.cpp
- examples/wa.cpp
- examples/ce.cpp
- examples/re.cpp
- examples/tle.cpp

## 5. 回归测试

    echo "===== AC test ====="
    ./minijudge examples/ac.cpp tests/input.txt tests/answer.txt

    echo "===== WA test ====="
    ./minijudge examples/wa.cpp tests/input.txt tests/answer.txt

    echo "===== CE test ====="
    ./minijudge examples/ce.cpp tests/input.txt tests/answer.txt

    echo "===== RE test ====="
    ./minijudge examples/re.cpp tests/input.txt tests/answer.txt

    echo "===== TLE test ====="
    ./minijudge examples/tle.cpp tests/input.txt tests/answer.txt

## 6. 当前技术点

- C++17
- fork 创建子进程
- execl / execlp 执行编译器和用户程序
- waitpid 等待子进程
- dup2 实现 stdin / stdout / stderr 重定向
- kill 实现超时终止
- setrlimit 禁用 core dump
- filesystem 创建临时目录
- chrono 统计运行时间

## 7. 临时文件

运行过程中会生成 judge_tmp 目录：

    judge_tmp/
    ├── main
    ├── output.txt
    ├── error.txt
    └── compile_error.txt

该目录不会提交到 Git。

## 8. 当前阶段限制

当前版本只是 Phase 1，还不是安全沙盒。

目前尚未实现：

- Docker 隔离
- 内存限制
- 输出大小限制
- 多测试点
- HTTP API
- Web 页面

当前版本只适合运行自己写的测试代码，不适合公开运行陌生人的代码。
