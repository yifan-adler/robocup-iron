# AGENTS.md

## 目标与优先级

本仓库当前目标是让四名队员都能独立复现官方平台和 Iron 四种比赛模式，然后再依据技能问询分配模块。优先级顺序：

1. 原始证据可追溯；
2. 环境可重复；
3. 基线可运行、可计分；
4. 题库格式和语义可靠；
5. 小步修正确性；
6. 最后才做得分策略优化。

## 不可破坏的边界

- `archive/legacy-2025/` 是只读历史证据，不得编辑、格式化或覆盖。
- `.work/planner-2025/` 是官方平台解压源；不得把队伍策略直接写入其中。
- `src/iron/` 的导入状态由 `legacy-stage2-2025` 标签保留。未取得四模式基线前，不大规模拆分 `rdfw.cpp`。
- 不提交 `.work/`、`artifacts/`、构建产物、日志、core dump 或个人 IDE 配置。
- 不引入 ROS、视觉、深度学习、CUDA 或 GPU 依赖，除非新规则或平台明确要求。

## 代码规范

- 保持 C++11 兼容，不擅自升级官方 SDK、Boost ABI 或语言标准。
- 源码和脚本不得保存个人绝对路径；运行时由脚本计算仓库路径。
- 修复和策略改动分开提交；一次提交只解决一类行为。
- 避免对 15 万字节的 `rdfw.cpp` 做无关格式化，保证 diff 可审查。
- 新增重试、递归或服务器交互必须有明确上限、超时和失败退出。
- 物体 ID、位置 ID、容器 ID 不得在没有显式查找或类型说明时互换。

## 必跑命令

修改环境或构建脚本：

```bash
./scripts/setup-ubuntu18.sh
./scripts/doctor.sh
./scripts/onboard.sh
```

修改题库校验器：

```bash
python3 -m unittest tools.test_validate_questions -v
python3 tools/validate_questions.py tests/problems
```

修改 Iron 行为：

```bash
./scripts/build.sh --client iron
./scripts/run_suite.sh --manifest tests/manifests/smoke.csv
```

取得正式回归清单后，还必须运行全量清单并对比逐题 `summary.json`。

## 变更验收

- 报告测试命令、退出码和日志目录，不只写“能跑”。
- 策略变更必须比较逐题原始分、官方封顶分、动作数、Ask/Sense 次数、耗时和超时。
- 出现新崩溃、挂死、无法解释的轨迹差异或总分回退时，不合并；先保留复现题。
- 每个比赛题必须有不同的作者和复核者，`question-review.csv` 状态为 `approved` 才能进入提交集合。
- 自动校验通过不代表语义正确；初始状态/约束、IT/NT 等价和可执行性仍需官方平台与人工审查。

## Git 约定

- 主分支为 `main`；无远程期间仍使用短生命周期分支。
- 分支建议：`env/*`、`fix/*`、`test/*`、`strategy/*`、`questions/*`。
- 提交信息建议使用 `chore:`、`docs:`、`test:`、`fix:`、`feat:`。
- 不改写或移动 `legacy-stage2-2025` 标签。
- 比赛冻结标签只能指向工作区干净、复现文档和回归结果齐全的提交。

## 团队协作

- 当前不按专业或猜测分配角色。先让每个人完成 `docs/onboarding/README.md` 中的同一套验收。
- 完成 `docs/team-skill-survey.md` 后再确定环境、C++、解析、runner、出题和现场负责人。
- 任何关键模块至少两人能构建和定位基本故障，避免单点依赖。
