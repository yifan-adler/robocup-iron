# Iron 客户端代码阅读指南与后续改进路线

## 1. 报告目的

本文用于帮助后续成员快速读懂 Iron 客户端，并在保留 2025 年有效能力的基础上继续优化。

当前原则是：

1. 先理解和验证现有行为，再修改策略。
2. 优先修复明确的正确性和超时风险。
3. 每次只改一类逻辑，并与当前 2026 基线逐题比较。
4. 不以“代码看起来更合理”作为完成标准，而以动作轨迹、官方得分、运行时间和稳定性作为标准。

当前客户端已经能够在 2026 环境运行，并通过仓库现有 Stage1、Stage2 共 100 个测试组合。后续优化应以此作为回归基线。

## 2. 代码总体结构

Iron 客户端的主要代码位于 `src/iron/`：

| 文件 | 作用 | 阅读优先级 |
|---|---|---:|
| [`main.cpp`](../src/iron/main.cpp#L10) | 创建 `RDFW`，初始化并进入官方平台事件循环 | 1 |
| [`rdfw.hpp`](../src/iron/rdfw.hpp#L305) | 对象、任务、约束、状态和主要函数声明 | 2 |
| [`rdfw.cpp`](../src/iron/rdfw.cpp#L249) | 环境解析、约束处理、规划、执行、感知和状态更新 | 1 |
| [`parser.cpp`](../src/iron/parser.cpp#L96) | 自然语言任务解析 | 4 |
| [`parser.hpp`](../src/iron/parser.hpp) | 自然语言解析器的数据结构和接口 | 4 |
| [`debuglog.hpp`](../src/iron/debuglog.hpp#L10) | 调试日志宏 | 3 |
| [`CMakeLists.txt`](../src/iron/CMakeLists.txt#L17) | 客户端编译目标和编译参数 | 3 |

`rdfw.cpp` 是主体代码，但不建议从第一行顺序读到最后一行。应按照实际调用链分层阅读。

## 3. 实际运行调用链

### 3.1 程序入口

入口在 [`main.cpp`](../src/iron/main.cpp#L10)：

```cpp
auto rdfw = make_shared<_home::RDFW>();
rdfw->Init(argc, argv);
rdfw->Run();
```

这里的 `Run()` 不是 Iron 自己实现的，而是继承自官方 `Plug`。官方平台负责连接评测服务器、接收环境与任务，然后回调 Iron 的 `Plan()`。

### 3.2 客户端总流程

主体入口是 [`RDFW::Plan()`](../src/iron/rdfw.cpp#L249)，流程可以概括为：

```text
重置本题状态
  ↓
解析环境 GetEnvDes()
  ↓
解析任务、补充信息和约束 GetTaskDes()
  ↓
Stage2 约束反推和环境纠错
  ↓
生成约束风险表
  ↓
任务排序与优化
  ↓
主任务循环
  ↓
剩余任务检查
  ↓
多 GOTO 聚合
  ↓
向平台结束本题
```

关键函数如下：

| 阶段 | 函数 |
|---|---|
| 环境解析 | [`ParseEnv()`](../src/iron/rdfw.cpp#L3549)、[`ParseEnvSentence()`](../src/iron/rdfw.cpp#L3426) |
| 指令解析 | [`ParseInstruction()`](../src/iron/rdfw.cpp#L3254)、[`ParseNaturalLanguage()`](../src/iron/rdfw.cpp#L3335) |
| 补充信息应用 | [`ParseInfo()`](../src/iron/rdfw.cpp#L3600) |
| 约束反推 | [`ApplyMustNearConstraintCorrection()`](../src/iron/rdfw.cpp#L4079)、[`ApplyMustInConstraintCorrection()`](../src/iron/rdfw.cpp#L4253)、[`ApplyOpenCloseCorrection()`](../src/iron/rdfw.cpp#L4349) |
| 约束表生成 | [`Cons_plan()`](../src/iron/rdfw.cpp#L584) |
| 任务与约束冲突过滤 | [`FilterConstraintsByTaskConflicts()`](../src/iron/rdfw.cpp#L687) |
| 任务排序 | [`TaskOptimization()`](../src/iron/rdfw.cpp#L805) |
| 主循环 | [`ExecuteMainTaskLoop()`](../src/iron/rdfw.cpp#L458) |
| 风险计算 | [`CalculateTaskRisk()`](../src/iron/rdfw.cpp#L899) |
| 动作类型分发 | [`SolveTask()`](../src/iron/rdfw.cpp#L1050) |
| 多 GOTO | [`ExecuteMultiGotoAggregation()`](../src/iron/rdfw.cpp#L1849) |
| 零动作检查 | [`ZeroActionPreCheck()`](../src/iron/rdfw.cpp#L2347) |
| Ask/Sense | [`AskLoc()`](../src/iron/rdfw.cpp#L2558)、[`SenseCurrentLocationOnly()`](../src/iron/rdfw.cpp#L2670) |
| 原子动作封装 | [`TakeOut()` 至 `Move()`](../src/iron/rdfw.cpp#L2975) |
| 题间清理 | [`Fini()`](../src/iron/rdfw.cpp#L3796) |

## 4. 推荐的代码阅读顺序

需要逐项点击源码时，使用独立导航文档：[`code-reading-order.md`](code-reading-order.md)。

### 第一遍：只理解主干

1. 阅读 `main.cpp`，确认客户端与官方平台的关系。
2. 阅读 `rdfw.hpp` 中 `Object`、`SmallObject`、`Container`、`Robot`、`Instruction` 和 `RDFW`。
3. 阅读 `RDFW::Plan()`，画出上述调用链。
4. 阅读 `ExecuteMainTaskLoop()` 和 `SolveTask()`，理解一项任务如何被选择和执行。
5. 阅读 `Move/PickUp/PutDown/Open/Close/PutIn/TakeOut`，理解动作成功后如何更新内部状态。

第一遍不要深入自然语言解析器，也不要尝试逐行理解多 GOTO。

### 第二遍：理解状态和约束

重点追踪以下字段：

- `objects`：所有对象，通常以对象 ID 为索引。
- `smallObjects`：全部小物体。
- `location`：机器人当前位置。
- `hold_id`、`plate_id`：手持和托盘上的物体。
- `tasks`：需要完成的任务。
- `not_infoConstrains`：必须不成立的信息约束。
- `not_taskConstrains`：禁止执行的任务约束。
- `notnot_infoConstrains`：必须成立的信息约束。
- `goto_cons`、`move_cons`、`pickup_cons` 等：约束风险查找表。

选择一个简单测试题，按“XML环境 → 解析后的对象 → 约束反推 → 风险表 → 动作日志”跟踪一遍，比脱离题目读代码更容易理解。

### 第三遍：理解特殊优化

按下列顺序阅读：

1. 约束反推。
2. 零动作检查。
3. 多 `puton`。
4. 多 `goto`。
5. `takeout` 纠错。
6. Ask/Sense 状态更新。

每读完一项，都应回答三个问题：

1. 它读取哪些状态？
2. 它可能修改哪些状态？
3. 修改后的状态由哪个函数继续使用？

## 5. “约束反推”的含义

2025 年代码总结中所说的“反推”，准确含义是“约束反推环境”。它不是根据环境反推出题目，也不是机器学习。

Stage2 的初始环境可能缺失或包含错误信息。客户端把 `must` 和 `must not` 约束当作额外证据，反过来修正内部保存的环境状态，然后再进行规划。

### 5.1 Must Near 反推

实现位于 [`ApplyMustNearConstraintCorrection()`](../src/iron/rdfw.cpp#L4079)。

当前逻辑包括：

1. 把必须 `near/on/nextto` 的对象连接成组。
2. 使用 `must not near` 排除明显错误的位置。
3. 对组内已知位置投票。
4. 多数位置获胜，平票时选择编号较小的位置。
5. 把组内对象的位置统一为选定位置。

例如 Stage2 第01题中，错误环境把红色罐子放在位置2，但约束要求它靠近位置7的白杯，同时不得靠近位置2的桌子。客户端因此把红色罐子的位置反推出7。

### 5.2 Must In 反推

实现位于 [`ApplyMustInConstraintCorrection()`](../src/iron/rdfw.cpp#L4253)。

- `must inside/in`：更新小物体的 `inside`、目标容器的内部物体列表，并让小物体位置等于容器位置。
- `must not inside/in`：若当前包含关系与约束冲突，则清除错误包含关系和位置。

### 5.3 Must Open/Closed 反推

实现位于 [`ApplyOpenCloseCorrection()`](../src/iron/rdfw.cpp#L4349)。

根据 `must opened/closed` 和 `must not opened/closed` 修正容器开关状态；当前约束冲突时默认设为关闭。

### 5.4 当前反推的局限

当前实现会直接覆盖 `objects[i]->location`、`inside` 或 `isOpen`，但没有记录该结论来自初始环境、约束、Ask、Sense 还是动作结果。因此：

- 无法区分强证据和弱证据。
- 新感知与旧推理冲突时，不容易决定相信谁。
- 一次错误投票可能污染后续风险计算和任务规划。
- `InferUnknownLocations()` 目前只有声明，没有实现或调用，真正生效的是三个 `Apply...Correction()` 函数。

## 6. 当前发现的重点问题

### 6.1 编译优化没有实际生效

[`CMakeLists.txt`](../src/iron/CMakeLists.txt#L17) 在创建 `example` 目标之后调用 `add_compile_options(-O2)`。当前实际生成的编译参数只有 `-std=gnu++11`，没有 `-O2`。

建议改为目标级配置：

```cmake
target_compile_options(example PRIVATE -O2 -Wall)
```

修改前后必须使用相同题目多次运行，以中位数比较时间，不能只比较单次结果。

### 6.2 正式运行仍开启大量调试输出

[`debuglog.hpp`](../src/iron/debuglog.hpp#L10) 无条件定义了 `__DEBUG__`。`rdfw.cpp` 中存在大量 `LOG`、`cout` 和多次完整环境输出，这些操作都发生在官方计时范围内。

建议区分：

- 比赛模式：只输出错误和最终摘要。
- 调试模式：保留详细状态和推理过程。

### 6.3 重试和时间边界不统一

- [`AskLoc()`](../src/iron/rdfw.cpp#L2558) 在得到 `not_known` 时可以无限询问。
- `open/close/putin/takeout/goto` 各自实现重试，规则不一致。
- [`SolveTask_Open()`](../src/iron/rdfw.cpp#L1418) 和 [`SolveTask_Close()`](../src/iron/rdfw.cpp#L1467) 的计数变量未初始化。

建议建立统一的重试控制器：限制次数，同时检查本题剩余时间。

### 6.4 对象 ID 与位置 ID 混用

部分代码默认“大物体 ID 等于位置编号”，例如 [`Sense()`](../src/iron/rdfw.cpp#L2597) 使用 `objects[location]`。多 GOTO 中也存在把选定位置传给要求对象 ID 的函数的路径。

当前测试题经常满足这种编号规律，因此问题容易被掩盖。后续应显式区分：

```cpp
using ObjectId = unsigned int;
using LocationId = int;
```

并维护 `location -> objects` 映射，禁止用位置直接索引 `objects`。

### 6.5 动态数组只解决了部分越界问题

[`EnsureLocationCapacity()`](../src/iron/rdfw.cpp#L992) 同时处理对象维度和位置维度，但二维表的行列含义并不统一。扩展行数不等于扩展所有列，较大的对象 ID 或位置 ID 仍可能越界。

建议拆分：

- `EnsureObjectCapacity(ObjectId)`；
- `EnsureLocationCapacity(LocationId)`；
- `EnsureObjectLocationMatrix(ObjectId, LocationId)`；
- `EnsureObjectObjectMatrix(ObjectId, ObjectId)`。

### 6.6 风险判断依赖固定魔法数字

主循环使用 [`risk >= 2`](../src/iron/rdfw.cpp#L511) 直接跳过任务，多 GOTO 中又出现 `<2`、`<3` 和其他阈值。这些阈值没有与官方任务分、约束分、动作成本和时间分统一。

建议把判断改为预计收益：

```text
预计收益 = 新完成任务分
         + 新保留约束分
         - 被破坏约束分
         - 动作成本
         - 超时风险
```

只有预计收益为正且能在剩余时间内完成时才执行。

### 6.7 零动作检查并非真正零动作

[`ZeroActionPreCheck()`](../src/iron/rdfw.cpp#L2347) 的注释表示不移动，但其核验函数可能调用 `Move()`。这会使“避免动作”优化反而产生动作和时间消耗。

建议拆分为：

1. `IsSatisfiedFromTrustedState()`：纯读取，不执行任何平台动作。
2. `VerifyWithBudget()`：明确记录允许使用的 Ask、Sense 和 Move 预算。

### 6.8 感知缓存可能过期

[`SenseCurrentLocationOnly()`](../src/iron/rdfw.cpp#L2670) 使用 `posSensedFlag` 避免重复感知。但拿起、放下、放入、取出物体后，位置内容已经变化，旧感知标记没有统一失效。

建议使用位置版本号，任何改变该位置内容的成功动作都递增版本，缓存只在版本一致时有效。

### 6.9 状态清理存在不必要的重新分配

[`Fini()`](../src/iron/rdfw.cpp#L3796) 多次调用 `shrink_to_fit()`，随后下一题又重新扩展数组。虽然 `Fini()` 本身在本题计时结束后执行，但重新分配发生在后续题目的运行阶段，可能增加批量测试耗时波动。

建议 `clear()` 后保留合理容量，只在客户端退出时统一释放。

## 7. 对应2025年12项优化的后续方向

| 2025年已有能力 | 后续改进方向 |
|---|---|
| 初始化和重置 | 配置与单题状态分离；保留数组容量；建立统一 `ResetForNextTest()` |
| 错误物体容错 | 区分唯一、歧义、缺失匹配；歧义时询问或安全跳过 |
| 约束反推 | 引入事实来源、可信度和冲突解释；避免直接无记录覆盖 |
| 单约束与多任务冲突 | 用动作效果模拟和预计得分替代固定冲突阈值 |
| 多 GOTO | 以完成目标数、动作数、约束损失和时间共同选择汇合位置 |
| 多 PutOn | 对同一目标只定位和感知一次，统一安排拿取与放置顺序 |
| 大位置编号 | 分离对象和位置维度，完整扩展二维表行列 |
| 零动作任务 | 使用可信状态纯判断；核验动作单独受预算控制 |
| 感知更新 | 合并多套 Sense 逻辑；动作后使相关缓存失效 |
| TakeOut | 使用状态机和枚举结果替代数字返回码1/2/3/4 |
| GOTO错误小物体 | 保存候选位置；失败后排除旧候选，禁止重复前往同一错误位置 |
| 定时器 | 使用 `steady_clock` 建立内部截止时间，提前停止低收益动作 |

## 8. 推荐的实施顺序

### 第一阶段：低风险时间优化

1. 让 `-O2` 实际生效。
2. 增加比赛日志等级，关闭详细调试输出。
3. 删除题间重复的 `shrink_to_fit()`。
4. 使用相同用例至少重复3次，比较耗时中位数。

这阶段原则上不改变动作轨迹。

### 第二阶段：稳定性修复

1. 初始化所有重试计数器。
2. 给所有 Ask 和 `while(1)` 增加次数及时间上限。
3. 在索引前统一检查 `UNKNOWN`、对象范围和位置范围。
4. 分离对象 ID 与位置 ID。
5. 修复二维动态数组的行列扩展。

这阶段目标是减少崩溃、无限循环和临近5秒的危险用例。

### 第三阶段：升级约束反推

建议把位置、包含关系和开关状态改为带证据的事实：

```cpp
enum class FactSource {
    InitialEnvironment,
    Info,
    ConstraintInference,
    Ask,
    Sense,
    SuccessfulAction
};

struct LocationFact {
    LocationId value;
    FactSource source;
    int confidence;
    unsigned version;
};
```

建议优先级：

```text
成功动作和直接 Sense
    > 可验证的硬约束推理
    > Ask
    > 初始环境描述
```

反推时不直接删除旧事实，而是保留候选及冲突原因，选择违反硬约束最少、证据最强的状态。

### 第四阶段：得分驱动规划

统一 `CalculateTaskRisk()`、`IsKeepingGoing()` 和多 GOTO 中的风险算法，形成一个动作效果模拟器：

1. 输入当前可信状态和候选任务。
2. 生成完成任务所需动作序列。
3. 模拟动作对任务和约束的影响。
4. 估计动作数量及时间。
5. 选择预计得分最高的任务或任务组合。

多 GOTO 和多 PutOn 应建立在该模拟器上，而不是继续增加独立的特殊判断。

### 第五阶段：拆分与可测试化

在行为稳定后，再逐步把 `rdfw.cpp` 拆分为：

```text
world_state.*       环境状态与事实来源
instruction.*       任务和约束模型
inference.*         约束反推
risk_model.*        动作效果与得分估计
planner.*           任务选择和组合规划
executor.*          原子动作及状态更新
perception.*        Ask/Sense及缓存
```

不要一次性重写。每移动一个模块，都必须保证逐题动作和得分可解释。

## 9. 修改后的验证标准

每个补丁至少执行以下检查：

1. 编译成功，并确认编译参数符合预期。
2. Stage1 20题和 Stage2 30题的 IT、NT 全部正常退出。
3. 无超时、崩溃、缺失分数和非零客户端退出码。
4. 比较修改前后的每题动作序列。
5. 对动作变化的题逐题解释原因。
6. 每个性能用例至少运行3次，使用中位数比较，避免把系统波动当成优化效果。
7. 总分提高不能以个别题失控或接近5秒超时为代价。

建议为每个历史优化点增加至少一道专门回归题：

- 错误位置 + must near；
- must in / must not in；
- must open / must closed；
- 对象 ID 与位置 ID 不相等；
- 位置编号大于100；
- Ask 连续返回 `not_known`；
- 多 GOTO 汇合；
- 多 PutOn；
- TakeOut 误导状态；
- 感知后返回旧位置；
- 零动作已完成与伪完成；
- 接近时间上限时主动停止。

## 10. 总结

Iron 当前最有价值的能力是：利用任务和约束修正不可靠环境，再以约束风险控制动作。后续优化应保留这个核心方向，但把它从“直接覆盖状态和经验阈值”升级为：

```text
带来源的环境事实
    + 可解释的约束推理
    + 有时间预算的状态核验
    + 按官方得分选择动作
```

近期最优先事项是编译与日志优化、重试边界、ID与位置分离以及越界防护；完成这些基础工作后，再升级反推和多任务规划，收益更稳定，回归风险也更低。
