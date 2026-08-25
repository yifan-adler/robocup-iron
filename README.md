# RoboCup Iron 复现仓库

本仓库用于复现和接管武汉理工大学 2025 服务机器人仿真项目的 Iron/RDFW 客户端，并为 2026 校赛建立可重复的环境、题库校验和回归流程。

当前最优先目标不是分工或改策略，而是让四名队员都能在自己的电脑上完成同一套验收：构建容器、启动官方平台、编译 Iron、跑完 Stage1/Stage2 × IT/NT 四种 smoke case。

## 当前状态

- Git 主分支：`main`
- 临时基线：2025 国赛 Stage2 候选包，详见 [src/iron/BASELINE.md](src/iron/BASELINE.md)
- 目标运行时：`linux/amd64`、Ubuntu 18.04 容器
- 宿主系统：Ubuntu 22.04、Windows + WSL2 均可
- 原始资料：只读保存在 `archive/legacy-2025/`
- 校赛硬截止：2026-09-10；计划 09-08 冻结、09-09 彩排
- 角色分配：待完成技能问询后确定，当前不按专业预设能力

## 五分钟入口

先安装 Docker Engine 或 Docker Desktop，并确认以下命令可用：

```bash
docker version
docker compose version
```

然后在仓库根目录执行：

```bash
docker compose build
docker compose run --rm dev ./scripts/onboard.sh
```

`onboard.sh` 会依次校验平台归档、构建官方客户端、运行一个官方 smoke、构建 Iron，并运行四模式 smoke。第一次构建需要下载 Ubuntu 镜像和软件包。

如果想分步定位问题：

```bash
docker compose run --rm dev ./scripts/bootstrap.sh
docker compose run --rm dev ./scripts/build.sh --client official
docker compose run --rm dev ./scripts/run_case.sh --stage 1 --mode it --case 01 --client official
docker compose run --rm dev ./scripts/build.sh --client iron
docker compose run --rm dev ./scripts/run_suite.sh --manifest tests/manifests/smoke.csv
```

运行结果位于 `artifacts/runs/`，每次运行独立建目录，并包含 `server.log`、`client.log` 和 `summary.json`。

## 四种固定模式

| 模式 | 服务端 | Iron 参数 |
|---|---|---|
| Stage1 IT | `-mode it` | `-nlp 0 -err 0 -ask_2 0 -stage 1` |
| Stage1 NT | `-mode nt` | `-nlp 1 -err 0 -ask_2 0 -stage 1` |
| Stage2 IT | `-mode it` | `-nlp 0 -err 1 -ask_2 1 -stage 2` |
| Stage2 NT | `-mode nt` | `-nlp 1 -err 1 -ask_2 1 -stage 2` |

这些参数由 `config/modes/` 管理，不要在个人脚本里复制另一套。

## 题库校验

```bash
docker compose run --rm dev python3 tools/validate_questions.py tests/problems
```

冻结 30 题前使用严格模式：

```bash
docker compose run --rm dev python3 tools/validate_questions.py tests/problems \
  --expected-per-stage 30 \
  --review-manifest tests/manifests/question-review.csv \
  --require-review \
  --json artifacts/question-validation.json
```

自动校验不能替代官方平台和第二人的语义审查；每个 XML 都会保留一项人工检查提醒。

## 仓库结构

```text
Robocup/
├─ README.md                         # 项目入口、运行命令与风险说明
├─ AGENTS.md                         # 代码边界、测试要求与协作规范
├─ compose.yaml                      # 统一开发容器入口
├─ .gitignore                        # 构建、日志和个人环境忽略规则
├─ .gitattributes                    # 文本换行与二进制文件属性
├─ src/
│  └─ iron/                          # 当前可开发的 Iron/RDFW 客户端
│     ├─ BASELINE.md                 # 基线来源、哈希与导入说明
│     └─ CMakeLists.txt              # 客户端构建入口
├─ infra/
│  └─ docker/
│     └─ Dockerfile                  # linux/amd64、Ubuntu 18.04 环境
├─ config/
│  └─ modes/                         # Stage1/2 × IT/NT 四种固定参数
├─ scripts/
│  ├─ doctor.sh                      # 宿主与容器依赖诊断
│  ├─ bootstrap.sh                   # 校验并解压官方平台
│  ├─ build.sh                       # 构建 official 或 iron 客户端
│  ├─ run_case.sh                    # 运行单题并保存独立结果
│  ├─ run_suite.sh                   # 按 manifest 批量回归
│  └─ onboard.sh                     # 新队员四模式一键验收
├─ tools/
│  ├─ validate_questions.py          # 题库格式与语义静态校验
│  ├─ test_validate_questions.py     # 校验器单元测试
│  └─ summarize_run.py               # 汇总逐题运行结果
├─ tests/
│  ├─ problems/
│  │  ├─ stage1/                     # 2026 Stage1 自出题
│  │  └─ stage2/                     # 2026 Stage2 自出题
│  ├─ manifests/                     # smoke 清单与题目复核记录
│  ├─ legacy/                        # 从历史题库筛出的固定回归题
│  └─ quarantine/                    # 有争议或暂时失败的题目
├─ docs/
│  ├─ onboarding/                    # 全员跑通步骤与成员记录模板
│  ├─ rules/                         # 2025/2026 规则及赛事通知副本
│  ├─ environment-and-reproduction.md
│  ├─ rules-diff-2025-2026.md
│  ├─ team-plan-17-days.md
│  └─ team-skill-survey.md
├─ archive/
│  └─ legacy-2025/                   # 上届原始证据，只读、禁止修改
├─ .work/                            # 平台解压与构建缓存，不进入 Git
└─ artifacts/                        # 运行日志、得分与汇总，运行时生成
```

日常开发主要修改 `src/iron/`、`scripts/`、`tools/`、`tests/` 和 `docs/`。`archive/legacy-2025/` 只用于追溯，`.work/` 与 `artifacts/` 都属于可重新生成的本地内容，不应提交。

## 进一步阅读

- [环境配置与复现指南](docs/environment-and-reproduction.md)
- [2025/2026 规则差异](docs/rules-diff-2025-2026.md)
- [全员跑通优先的 17 天计划](docs/team-plan-17-days.md)
- [队员技能问询模板](docs/team-skill-survey.md)
- [代理与协作约束](AGENTS.md)

## 已知风险

- 候选包尚未被上届确认成正式提交版本。
- 当前宿主机没有 Docker/WSL，因此仓库内已完成静态测试，但完整 Linux 构建与仿真需由装好 Docker 的机器验证。
- 历史代码存在任务索引悬空指针、未初始化重试计数、无界 AskLoc、ID/位置混用和 multi-goto 风险；必须先冻结分数基线再逐项修复。
- 2026 规则要求同题不得重复任务和约束，少题或格式错误可能每题扣 500 分，且负分不清零。
