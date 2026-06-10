# MiniJudge

MiniJudge 是一个用 C++ 实现的轻量级在线判题核心 / 代码运行沙盒。

当前版本支持：

- Phase 1：单测试点命令行判题
- Phase 2：多测试点判题
- Phase 3：资源限制与进程控制
- Phase 4：答案比较器增强
- Phase 5：判题配置系统

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
- examples/output_limit.cpp
- examples/memory_limit.cpp
- examples/fork_limit.cpp
- examples/no_newline.cpp

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

资源限制与比较器测试：

    ./minijudge examples/output_limit.cpp tests/input.txt tests/answer.txt
    ./minijudge examples/memory_limit.cpp tests/input.txt tests/answer.txt
    ./minijudge examples/fork_limit.cpp tests/input.txt tests/answer.txt
    ./minijudge examples/no_newline.cpp tests/input.txt tests/answer.txt

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
- setrlimit 进行资源限制
- setpgid + kill(-pid) 进行进程组清理
- 逐字符答案比较
- filesystem 创建临时目录和检测测试点
- chrono 统计运行时间

## 10. Phase 3：资源限制与进程控制

### 已实现能力

当前 `runner.cpp` 已经加入以下限制与控制逻辑：

- `RLIMIT_CORE`：禁用 core dump
- `RLIMIT_FSIZE`：限制输出文件大小
- `RLIMIT_AS`：限制虚拟内存使用
- `RLIMIT_NPROC`：限制进程数量
- `setpgid`：让用户程序进入独立进程组
- `kill(-pid, SIGKILL)`：超时时杀死整个进程组

这样可以防止用户程序：

- 无限输出撑爆磁盘
- 大量申请内存
- 疯狂 fork 创建进程
- 死循环长期占用 CPU
- 崩溃后生成 core 文件
- fork 后留下子进程残留

### 当前资源限制参数

当前代码中的默认参数为：

- 时间限制：`2000 ms`
- 输出限制：`1 MB`
- 内存限制：`128 MB`
- 进程数量限制：`16`
- 轮询间隔：`1 ms`

对应实现位置：

- [src/main.cpp](/home/wyh/MiniJudge/src/main.cpp:42)
- [src/runner.cpp](/home/wyh/MiniJudge/src/runner.cpp:13)

## 11. Phase 4：答案比较器增强

### 已实现能力

比较器已经从简单字符串比较升级为：

- 逐字符比较
- 遇到第一个不同字符立即判定 `WA`
- 记录第一个错误位置
- 输出行号和列号
- 忽略文件末尾空白字符

例如输出为：

    -

而标准答案为：

    3

会得到类似结果：

    Compile: OK
    Result: WA
    Compare failed at line 1, column 1
    Output char: '-'
    Answer char: '3'
    Time: 10 ms

多测试点模式下，也会在首个错误测试点输出相同的定位信息。

### 忽略末尾空白

当前会把以下两种输出视为一致：

    3

和：

    3\n

对应测试程序：

- `examples/no_newline.cpp`

对应实现位置：

- [src/comparator.cpp](/home/wyh/MiniJudge/src/comparator.cpp:1)

## 12. Phase 5：判题配置系统

### 架构调整

本阶段将判题参数从代码中抽离，形成统一配置结构：

```cpp
struct JudgeConfig {
    int timeLimitMs = 2000;
    int waitIntervalMs = 1;

    int cpuLimitSeconds = 2;
    int outputLimitMb = 1;
    int memoryLimitMb = 128;
    int processLimit = 16;
};
```

当前判题逻辑不再依赖硬编码参数，而是统一从 `JudgeConfig` 读取。

整体结构：

```text
problem.conf
      ↓
config_loader
      ↓
JudgeConfig
      ↓
runner
```

---

### 新增文件

```text
include/judge_config.h
include/config_loader.h
src/config_loader.cpp
```

---

### 题目配置文件

当前支持：

```text
problems/1001/problem.conf
```

示例：

```text
time_limit_ms=2000
wait_interval_ms=1
cpu_limit_seconds=2
output_limit_mb=1
memory_limit_mb=128
process_limit=16
```

运行：

```text
./minijudge examples/ac.cpp problems/1001
```

时会自动读取配置文件。

读取成功：

```text
Config: loaded from problems/1001/problem.conf
```

读取失败：

```text
Config: using default limits
```

---

### 配置解析鲁棒性

当前配置系统支持：

- 忽略空行
- 忽略注释行
- 忽略未知配置项
- 非法数字自动跳过
- 非正数自动忽略
- 保留默认配置

例如：

```text
memory_limit_mb=abc
process_limit=-1
```

不会导致程序崩溃。

## 13. 当前项目能力总结

MiniJudge 当前已经支持：

- 单测试点判题
- 多测试点判题
- `AC / WA / CE / RE / TLE`
- 编译错误捕获
- 运行错误判断
- 超时判断
- 输出重定向
- 错误重定向
- 运行时间统计
- 基础资源限制
- 进程组清理
- 逐字符答案比较
- `WA` 错误位置提示
- 忽略末尾空白
- 判题配置文件加载
- 默认配置回退

## 14. 临时文件

运行过程中会生成 judge_tmp 目录：

    judge_tmp/
    ├── main
    ├── output.txt
    ├── error.txt
    └── compile_error.txt

该目录不会提交到 Git。

## 15. 当前阶段限制

当前版本还不是安全沙盒。

目前尚未实现：

- Docker 隔离
- cgroup 资源控制
- seccomp 系统调用过滤
- 用户权限隔离
- 文件系统隔离
- 网络隔离
- Special Judge
- 浮点误差比较
- HTTP API
- Web 页面

当前版本只适合运行自己写的测试代码，不适合公开运行陌生人的代码。

## 16. 后续建议

下一阶段可以进入：

- Phase 6：Docker 沙盒隔离

当前最大的限制已经不是功能不足，而是安全性不足。

虽然已经支持：

- 输出限制
- 内存限制
- CPU 限制
- 进程数量限制
- 进程组清理

但用户代码仍然直接运行在宿主机上。

下一阶段目标：

- Docker 容器隔离
- 容器资源限制
- 容器文件系统隔离
- 为后续 Web OJ 做准备
