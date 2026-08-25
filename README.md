# RoboCup Iron 复现仓库

本仓库用于复现和接管武汉理工大学 2025 服务机器人仿真项目的 Iron/RDFW 客户端，并为 2026 校赛建立可重复的环境、题库校验和回归流程。

当前首要目标是让四名队员都能在 Windows + WSL2 + Ubuntu 18.04 中完成同一套验收：启动官方平台、编译 official 和 Iron、跑完 Stage1/Stage2 × IT/NT 四种 smoke case。全员跑通后再依据技能问询分工。

## 当前状态

- 主分支：`main`
- 临时基线：2025 国赛 Stage2 候选包，详见 `src/iron/BASELINE.md`
- 目标运行时：WSL2、Ubuntu 18.04、x86_64、C++11
- 原始资料：只读保存在 `archive/legacy-2025/`
- 校赛硬截止：2026-09-10；计划 09-08 冻结、09-09 彩排

## 首次环境安装

在 Windows PowerShell 中运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install-wsl-ubuntu1804.ps1
```

脚本安装独立的 `Ubuntu-18.04` 发行版。首次启动窗口中创建 Linux 用户和密码，退出后进入该发行版：

```powershell
wsl -d Ubuntu-18.04
```

仓库应放在 WSL 的 Linux 文件系统，例如 `/home/<用户名>/robocup-iron`。如果尚未能在 WSL 中克隆私有仓库，可先把 Windows 上的完整仓库复制进去：

```bash
cp -a /mnt/d/path/to/Robocup ~/robocup-iron
cd ~/robocup-iron
```

不要长期在 `/mnt/c` 或 `/mnt/d` 中编译。进入 Linux 仓库后执行：

```bash
./scripts/setup-ubuntu18.sh
./scripts/doctor.sh
./scripts/onboard.sh
```

只有 `apt-get update` 因 Bionic 软件源缺失或失效而失败时，才显式修复源：

```bash
./scripts/setup-ubuntu18.sh --repair-sources
```

## 分步运行

```bash
./scripts/bootstrap.sh
./scripts/build.sh --client official
./scripts/run_case.sh --stage 1 --mode it --case 01 --client official
./scripts/build.sh --client iron
./scripts/run_suite.sh --manifest tests/manifests/smoke.csv
```

运行结果位于 `artifacts/runs/`；每次运行独立创建目录，包含 `server.log`、`client.log`、`platform-log/` 和 `summary.json`。

## 四种固定模式

| 模式 | 服务端 | Iron 参数 |
|---|---|---|
| Stage1 IT | `-mode it` | `-nlp 0 -err 0 -ask_2 0 -stage 1` |
| Stage1 NT | `-mode nt` | `-nlp 1 -err 0 -ask_2 0 -stage 1` |
| Stage2 IT | `-mode it` | `-nlp 0 -err 1 -ask_2 1 -stage 2` |
| Stage2 NT | `-mode nt` | `-nlp 1 -err 1 -ask_2 1 -stage 2` |

参数由 `config/modes/` 管理，不要在个人脚本中维护第二套值。

## 题库校验

```bash
python3 -m unittest tools.test_validate_questions -v
python3 tools/validate_questions.py tests/problems
```

冻结 30 题前使用严格模式：

```bash
python3 tools/validate_questions.py tests/problems \
  --expected-per-stage 30 \
  --review-manifest tests/manifests/question-review.csv \
  --require-review \
  --json artifacts/question-validation.json
```

自动校验不能替代官方平台和第二人的语义审查。

## 仓库结构

```text
Robocup/
├─ AGENTS.md
├─ README.md
├─ archive/legacy-2025/       # 只读历史证据
├─ config/modes/              # 四种固定参数
├─ docs/                      # 环境、规则、计划与 onboarding
├─ infra/patches/             # 可审查的平台工作副本补丁
├─ scripts/                   # 安装、诊断、构建和 runner
├─ src/iron/                  # Iron/RDFW 客户端
├─ tests/                     # 题库、manifest 和固定回归题
├─ tools/                     # 校验和日志汇总工具
├─ .work/                     # 本地解压与构建缓存，不提交
└─ artifacts/                 # 本地运行产物，不提交
```

## 进一步阅读

- `docs/environment-and-reproduction.md`
- `docs/onboarding/README.md`
- `docs/rules-diff-2025-2026.md`
- `docs/team-plan-17-days.md`
- `docs/team-skill-survey.md`

## 已知风险

- Ubuntu 18.04 已超出标准维护期，仅因官方平台 ABI 和比赛工具链要求继续使用；该发行版只用于本项目。
- 候选包尚未被上届确认成正式提交版本。
- 历史代码仍有任务索引悬空指针、未初始化重试计数、无界 AskLoc、ID/位置混用和 multi-goto 风险；先冻结分数基线再逐项修复。
- 2026 规则要求同题不得重复任务和约束，少题或格式错误可能每题扣 500 分，且负分不清零。
