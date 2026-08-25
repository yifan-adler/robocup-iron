# Iron 环境配置与复现指南

## 1. 目标

本指南用于让不同宿主系统上的队员得到相同的 Linux 运行边界：

```text
Ubuntu 22.04 或 Windows
          ↓ Docker
linux/amd64 Ubuntu 18.04
          ↓
官方 cserver + libasp + iclingo
          ↓
官方 example / Iron RDFW 客户端
          ↓
固定 XML、参数、日志和 summary.json
```

这个项目是离散规划仿真，不依赖 ROS、摄像头、底盘或 CUDA。2026 规则虽然列出 RTX 3060，但现有源码没有 GPU 调用，因此复现阶段不启用 GPU。

## 2. 宿主机准备

### Ubuntu 22.04

使用 Docker 官方 apt 仓库安装 Docker Engine、CLI、Buildx 和 Compose plugin，不要同时混装发行版的 `docker.io` 与 Docker CE。官方步骤：

- [Install Docker Engine on Ubuntu](https://docs.docker.com/engine/install/ubuntu/)

安装后验证：

```bash
sudo systemctl status docker
sudo docker run --rm hello-world
docker compose version
```

若普通用户没有权限，可按 Docker 官方 post-install 指南加入 `docker` 组，然后重新登录。加入 `docker` 组等价于授予较高本机权限，只对受信任队员电脑这样设置。

### Windows 10/11

1. 以管理员身份打开 PowerShell，执行 `wsl --install` 并重启；要求 Windows 10 2004/Build 19041 以上或 Windows 11。
2. 安装 Docker Desktop，选择 WSL2 backend，并为所用的 WSL 发行版开启 integration。
3. 在 PowerShell 或 WSL 中验证 `docker version` 和 `docker compose version`。

官方入口：

- [Install WSL](https://learn.microsoft.com/windows/wsl/install)
- [Install Docker Desktop on Windows](https://docs.docker.com/desktop/setup/install/windows-install/)

不要求 Windows 队员安装 Ubuntu 18.04 发行版；Ubuntu 18.04 位于项目容器内。

## 3. 获取并检查仓库

当前尚未配置远程仓库。通过团队约定的方式取得完整仓库后，在根目录运行：

```bash
bash ./scripts/doctor.sh
git status --short --branch
```

`doctor.sh` 检查 Git、Docker、Compose daemon 和官方平台 ZIP 的 SHA256。不要从聊天软件再次解压或重新打包 `archive/legacy-2025/`。

如果在 Windows 上执行 Shell 脚本遇到权限或行尾问题，优先使用：

```powershell
docker compose run --rm dev bash ./scripts/bootstrap.sh
```

仓库通过 `.gitattributes` 固定 Shell/C++ 文件为 LF；不要把全仓库转换成 CRLF。

## 4. 构建统一镜像

```bash
docker compose build
```

镜像固定为 `linux/amd64` 和 Ubuntu 18.04，包含：

- GCC/G++ 与 Make；
- CMake；
- Boost 开发库；
- Python 3；
- unzip、coreutils 和进程诊断工具。

首次构建必须联网拉取基础镜像和软件包。如果 Ubuntu 18.04 的普通软件源不可用，Dockerfile 会回退到 `old-releases.ubuntu.com`。

规则文档写 gcc 4.6.5，但 Ubuntu 18.04 默认工具链并不是这一版本，历史安装说明还建议过 Boost 1.70。当前采取以下原则：

1. 先在 Ubuntu 18.04 标准包环境验证官方预编译 ABI；
2. 自动记录实际 GCC、CMake、Boost 和平台 ZIP 哈希；
3. 只有上届确认正式工具链或出现明确 ABI 错误时，才增加第二个兼容镜像；
4. 不为了“看起来更新”而升级官方 SDK。

## 5. 校验并解压官方平台

```bash
docker compose run --rm dev ./scripts/bootstrap.sh
```

脚本校验：

```text
Planner-release-2025(1).zip
SHA256 fdb9cf54054aaac0ccbfbeabc97a99d0067dc975f0999a90f81fe83a326ac150
```

解压目标为 `.work/planner-2025/`。目标已存在但缺少正确 stamp 时脚本会停止，不会覆盖未知内容。官方平台根目录应包含：

```text
bin/cserver
lib/libasp.so
res/iclingo
include/cserver/plug.hpp
tests/problems/{IT,NT}/{stage1,stage2}
```

## 6. 先验证官方 example

```bash
docker compose run --rm dev ./scripts/build.sh --client official
docker compose run --rm dev ./scripts/run_case.sh \
  --stage 1 --mode it --case 01 --client official
```

这一步失败时不要修改 Iron。按以下顺序定位：

1. Docker 镜像与 CPU 架构；
2. ZIP 哈希和解压结构；
3. GCC/CMake/Boost 配置；
4. 官方静态库和 `libasp.so` ABI；
5. `iclingo`、`.lp` 文件权限和工作目录；
6. cserver 端口或残留进程。

## 7. 构建 Iron

```bash
docker compose run --rm dev ./scripts/build.sh --client iron
```

构建脚本从只读解压源复制一个 `.work/platform-iron/` 工作副本，只把 `src/iron/` 的八个运行文件覆盖到工作副本的 `example/`。原始 ZIP、`archive/` 和解压源不会被修改。

环境记录写入：

```text
.work/environment/official.txt
.work/environment/iron.txt
```

## 8. 运行四种模式

单题：

```bash
docker compose run --rm dev ./scripts/run_case.sh \
  --stage 2 --mode nt --case 01 --client iron --timeout 60
```

四模式 smoke：

```bash
docker compose run --rm dev ./scripts/run_suite.sh \
  --manifest tests/manifests/smoke.csv
```

参数从 `config/modes/` 读取：

| Stage | Mode | mis/err/ans | `-nlp` | `-err` | `-ask_2` | `-stage` |
|---:|---|---|---:|---:|---:|---:|
| 1 | IT | off/off/off | 0 | 0 | 0 | 1 |
| 1 | NT | off/off/off | 1 | 0 | 0 | 1 |
| 2 | IT | on/on/on | 0 | 1 | 1 | 2 |
| 2 | NT | on/on/on | 1 | 1 | 1 | 2 |

## 9. 运行产物

每次运行创建：

```text
artifacts/runs/<UTC时间>-s<stage>-<mode>-<case>-<client>-<pid>/
├─ server.log
├─ client.log
├─ platform-log/
└─ summary.json
```

`summary.json` 至少包含：

- Stage、mode、题号和客户端；
- server/client 退出码；
- 是否超时；
- wall time；
- 原始分数；
- 2026 规则下封顶 1000 的官方分数，负分保持负数；
- 从 cserver 日志识别出的动作总数和分类。

日志解析不到分数时 `raw_score` 为 `null`，不能把它解释成 0 分。

## 10. 全员 onboarding

每名队员运行：

```bash
docker compose build
docker compose run --rm dev ./scripts/onboard.sh
```

然后复制 `docs/onboarding/member-template.md`，记录宿主版本、五次 smoke 结果和遇到的问题。四人都通过前不进行固定角色分配，也不开始高风险策略修改。

## 11. 出题与严格校验

日常校验：

```bash
docker compose run --rm dev python3 tools/validate_questions.py tests/problems
```

提交候选校验：

```bash
docker compose run --rm dev python3 tools/validate_questions.py tests/problems \
  --expected-per-stage 30 \
  --review-manifest tests/manifests/question-review.csv \
  --require-review \
  --json artifacts/question-validation.json
```

自动检查 XML、Stage 开关、机器人字段、ID 连续性、小物体颜色、IT 括号、顶层指令类型、重复任务/约束、NL 数量和句号、作者/复核者分离。以下内容必须再由官方平台和第二人确认：

- 初始状态不破坏约束；
- IT 与 NT 语义真正一致；
- 任务可执行且不存在歧义；
- Stage2 的 mis/err/extra/ans 合理；
- 理论分、实际分、超时和负分风险。

## 12. 常见故障

| 现象 | 优先检查 |
|---|---|
| `docker` 命令不存在 | 宿主 Docker/WSL 安装 |
| Docker daemon unavailable | Docker Desktop/`systemctl status docker` |
| Ubuntu 镜像拉取失败 | 网络、代理、Docker registry |
| ZIP hash mismatch | 文件传输或被重新打包；不要继续 |
| CMake 找不到 Boost | 镜像是否为本仓库构建、是否混用宿主构建 |
| `libasp.so` 加载失败 | 架构、ABI、路径、Linux loader |
| cserver 一直等待 | 客户端是否启动、旧进程、模式和工作目录 |
| NT 立即失败 | `-nlp 1`、`words.txt` 路径、NL 句号 |
| Stage2 挂死 | AskLoc/递归重试、错误回答序列、单题 timeout |
| 分数为空 | 先看 server/platform log，勿当成 0 分 |

## 13. 复现完成标准

- 至少两台不同宿主机从干净仓库完成 onboarding；
- 官方和 Iron 都能构建；
- Iron 四模式各跑一题，日志和 `summary.json` 齐全；
- `.work/environment/*.txt` 记录一致或差异可解释；
- 重复三次固定 smoke 无新增崩溃、挂死或不可解释轨迹差异；
- 第二名队员不依赖口头指导即可按本文档完成。
