# Iron 客户端顺序阅读导航

> 按程序实际运行顺序排列。点击每一步的函数名即可跳转到对应源码。

## 第一段：先看程序骨架

1. [`main()`：程序入口](../src/iron/main.cpp#L10)
   - 创建 `RDFW` 对象。
   - 调用 `Init()`。
   - 进入官方平台提供的 `Run()` 事件循环。

2. [`Object`：所有对象的基础类](../src/iron/rdfw.hpp#L64)
   - 先理解 `id`、`sort`、`location`。

3. [`SmallObject`：小物体状态](../src/iron/rdfw.hpp#L96)
   - 重点看 `color`、`inside`、`on`。

4. [`BigObject` 和 `Container`：大物体与容器](../src/iron/rdfw.hpp#L127)
   - 接着阅读 [`Container`](../src/iron/rdfw.hpp#L154)。
   - 重点看 `isOpen` 和 `smallObjectsInside`。

5. [`Robot`：机器人自身状态](../src/iron/rdfw.hpp#L209)
   - 重点看当前位置、手持物体和托盘物体。

6. [`Condition` 和 `Instruction`：任务表达方式](../src/iron/rdfw.hpp#L275)
   - 接着阅读 [`Instruction`](../src/iron/rdfw.hpp#L643)。
   - 理解 `behave`、`X`、`Y`、`risk`、`isEnable`。

7. [`RDFW`：整个客户端工作区](../src/iron/rdfw.hpp#L305)
   - 暂时只看成员变量分组和函数声明，不要逐行深挖。

## 第二段：按一题的实际运行顺序阅读

8. [`RDFW::Init()`：读取启动参数](../src/iron/rdfw.cpp#L147)
   - 理解 `stage`、自然语言模式、纠错模式和词典路径。

9. [`RDFW::Plan()`：单题总入口](../src/iron/rdfw.cpp#L249)
   - 这是最重要的导航函数。
   - 第一遍只沿着它调用的函数继续阅读。

10. [`ParseEnv()`：拆分环境描述](../src/iron/rdfw.cpp#L3549)
    - 接着阅读 [`ParseEnvSentence()`](../src/iron/rdfw.cpp#L3426)。
    - 看环境里的 `at/sort/size/color/inside/opened/closed` 如何写入对象。

11. [`ParseInstruction()`：解析结构化指令](../src/iron/rdfw.cpp#L3254)
    - 理解 IT 模式如何生成任务、普通信息和约束。

12. [`ParseNaturalLanguage()`：解析自然语言指令](../src/iron/rdfw.cpp#L3335)
    - 接着阅读 [`parser::parse()`](../src/iron/parser.cpp#L96)。
    - 第一遍只理解输入输出，不必深入语法树实现。

13. [`ParseInfo()`：把普通补充信息写回环境](../src/iron/rdfw.cpp#L3600)
    - 重点看 `on/near/inside/opened/closed` 对状态的修改。

## 第三段：重点阅读去年新增的约束反推

14. [`ApplyOpenCloseCorrection()`：反推容器开关](../src/iron/rdfw.cpp#L4349)
    - 根据 `must` 和 `must not` 修正 `isOpen`。

15. [`ApplyMustInConstraintCorrection()`：反推包含关系](../src/iron/rdfw.cpp#L4253)
    - 修正 `inside`、物体位置和容器内部列表。

16. [`ApplyMustNearConstraintCorrection()`：反推位置](../src/iron/rdfw.cpp#L4079)
    - 重点看排除错误位置、并查集合组、位置投票和状态覆盖。

17. [`Stage2 第01题`：结合实例理解反推](../tests/problems/stage2/01.xml#L3)
    - 对照错误位置、`must near` 和 `must not near` 阅读第16步。

## 第四段：理解任务如何被选择

18. [`Cons_plan()`：生成约束风险表](../src/iron/rdfw.cpp#L584)
    - 看约束如何变成 `goto_cons`、`move_cons`、`pickup_cons` 等计数。

19. [`FilterConstraintsByTaskConflicts()`：任务与约束冲突处理](../src/iron/rdfw.cpp#L687)
    - 理解哪些约束会被放弃以及当前使用的阈值。

20. [`TaskOptimization()`：任务排序](../src/iron/rdfw.cpp#L805)
    - 看不同动作类型的执行优先级。

21. [`CalculateTaskRisk()`：计算任务风险](../src/iron/rdfw.cpp#L899)
    - 接着阅读 [`CalculateStepRisk()`](../src/iron/rdfw.cpp#L983)。
    - 结合主循环里的 `risk >= 2` 判断理解任务为什么执行或跳过。

## 第五段：理解任务如何执行

22. [`ExecuteMainTaskLoop()`：主任务循环](../src/iron/rdfw.cpp#L458)
    - 看缺失对象、风险预判、零动作检查和任务执行顺序。

23. [`SolveTask()`：按动作类型分发](../src/iron/rdfw.cpp#L1050)
    - 从这里进入每种具体任务。

24. 按以下顺序阅读具体任务：
    1. [`SolveTask_PickUp()`](../src/iron/rdfw.cpp#L1311)
    2. [`SolveTask_PutDown()`](../src/iron/rdfw.cpp#L1324)
    3. [`SolveTask_Goto()`](../src/iron/rdfw.cpp#L1350)
    4. [`SolveTask_Open()`](../src/iron/rdfw.cpp#L1394)
    5. [`SolveTask_Close()`](../src/iron/rdfw.cpp#L1441)
    6. [`SolveTask_Give()`](../src/iron/rdfw.cpp#L1490)
    7. [`SolveTask_Putin()`](../src/iron/rdfw.cpp#L1509)
    8. [`SolveTask_TakeOut()`](../src/iron/rdfw.cpp#L1629)
    9. [`SolveTask_PutOn()`](../src/iron/rdfw.cpp#L1697)

25. [`IsZeroActionSatisfy()`：判断任务是否已经满足](../src/iron/rdfw.cpp#L2281)
    - 接着阅读 [`ZeroActionPreCheck()`](../src/iron/rdfw.cpp#L2347)。

26. [`GetSmallObjectStatus()`：询问小物体状态](../src/iron/rdfw.cpp#L2430)
    - 接着阅读 [`GetBigObjectStatus()`](../src/iron/rdfw.cpp#L2514) 和 [`AskLoc()`](../src/iron/rdfw.cpp#L2558)。

27. [`SenseCurrentLocationOnly()`：感知并更新当前位置](../src/iron/rdfw.cpp#L2670)
    - 再回看 [`Sense()`](../src/iron/rdfw.cpp#L2576)，比较两套感知逻辑。

## 第六段：理解客户端怎样调用官方动作

28. 按动作依赖顺序阅读原子动作封装：
    1. [`Move()`](../src/iron/rdfw.cpp#L3098)
    2. [`PickUp()`](../src/iron/rdfw.cpp#L3082)
    3. [`PutDown()`](../src/iron/rdfw.cpp#L3068)
    4. [`ToPlate()`](../src/iron/rdfw.cpp#L3055)
    5. [`FromPlate()`](../src/iron/rdfw.cpp#L3043)
    6. [`Open()`](../src/iron/rdfw.cpp#L3028)
    7. [`Close()`](../src/iron/rdfw.cpp#L3013)
    8. [`PutIn()`](../src/iron/rdfw.cpp#L2993)
    9. [`TakeOut()`](../src/iron/rdfw.cpp#L2975)

29. [`UpdateTaskList()`：动作成功后的任务状态更新](../src/iron/rdfw.cpp#L3699)
    - 检查动作成功后哪些任务被关闭、哪些约束表被清零。

## 第七段：最后阅读组合优化和收尾

30. [`CheckAndDeferMultiGoto()`：识别多 GOTO](../src/iron/rdfw.cpp#L1838)

31. [`ExecuteMultiGotoAggregation()`：多 GOTO 聚合](../src/iron/rdfw.cpp#L1849)
    - 最后再读这一段，因为它依赖对象状态、风险表、任务状态和原子动作。

32. [`ExecuteCheckPhase()`：重试剩余任务](../src/iron/rdfw.cpp#L546)

33. [`Fini()`：一题结束后的状态清理](../src/iron/rdfw.cpp#L3796)

## 阅读时的记录模板

每读一个函数，只记录下面五项：

```text
函数名：
谁调用它：
它读取什么状态：
它修改什么状态：
失败或返回 false 后去哪里：
```

第一轮读到第17步后，应该能够解释“环境错误时如何通过约束反推出正确状态”；读到第29步后，应该能够完整解释一个任务从解析到执行的全过程。
