# Iron：Windows + WSL2 + Ubuntu 18.04 环境配置与复现

## 1. 运行边界

```text
Windows 10/11 x64
        ↓ WSL2
Ubuntu 18.04 x86_64
        ↓
官方 cserver + libasp + iclingo
        ↓
official example / Iron RDFW
        ↓
固定 XML、参数、日志和 summary.json
```

本项目是离散规划仿真，不需要 ROS、视觉、CUDA 或 GPU。Ubuntu 18.04 用于匹配官方平台和预编译库的 ABI，不作为通用日常系统。

## 2. Windows 侧安装 Ubuntu 18.04

要求 Windows 已启用 WSL 和 Virtual Machine Platform，且 `wsl --status` 可运行。若尚未启用，请先按照 Microsoft 的 WSL 安装文档启用功能并重启。

在仓库根目录的 PowerShell 中执行：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install-wsl-ubuntu1804.ps1
```

可指定 APPX 保存位置：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install-wsl-ubuntu1804.ps1 `
  -DownloadDirectory D:\ubuntu1804
```

脚本会：

1. 保留所有已有 WSL 发行版和原默认发行版；
2. 下载固定的 Ubuntu 18.04 APPX；
3. 校验 SHA256 `0b1abe8d5dc3ff416a06c9524e4f61a1fdf6ced583cb9b297ee72df1732ff403`；
4. 安装并打开首次初始化窗口；
5. 确认 `Ubuntu-18.04` 为 x86_64 和 WSL2。

首次窗口要求创建 Linux 用户和密码。密码输入不会显示字符；完成后执行 `exit`，Windows 脚本会继续验证。

只检查已有安装而不做修改：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install-wsl-ubuntu1804.ps1 -CheckOnly
```

## 3. 把仓库放进 Linux 文件系统

进入发行版：

```powershell
wsl -d Ubuntu-18.04
```

推荐路径：

```text
/home/<用户名>/robocup-iron
```

已有 Windows 完整仓库时：

```bash
cp -a /mnt/d/path/to/Robocup ~/robocup-iron
cd ~/robocup-iron
```

也可以先安装 Git，再直接克隆：

```bash
sudo apt-get update
sudo apt-get install -y ca-certificates git
git clone https://github.com/yifan-adler/robocup-iron.git ~/robocup-iron
cd ~/robocup-iron
```

`/mnt/c` 和 `/mnt/d` 可用于传输，但不建议长期构建，原因是权限、性能、可执行位和文件系统语义可能不同。

## 4. 安装工具链

```bash
./scripts/setup-ubuntu18.sh
```

脚本拒绝非 Ubuntu 18.04、非 WSL2 或非 x86_64 环境，并安装固定依赖集合。重复运行是安全的。

默认不会覆盖 `/etc/apt/sources.list`。如果 `apt-get update` 明确因 Bionic 源缺失或错误而失败，检查当前文件后再执行：

```bash
./scripts/setup-ubuntu18.sh --repair-sources
```

该选项会先把现有文件备份为 `/etc/apt/sources.list.robocup-backup-<UTC时间>`，然后写入已验证的 `bionic`、`bionic-updates`、`bionic-security`、`bionic-backports` 源并重建索引。不要在普通网络错误、代理错误或 DNS 错误时盲目使用。

## 5. 环境诊断

```bash
./scripts/doctor.sh
```

诊断范围包括：

- Ubuntu 18.04、WSL2、x86_64；
- Git、GCC/G++、Make、CMake、Boost、Python、patch、unzip 和 timeout；
- Shell 脚本执行位与 CRLF；
- 官方平台 ZIP 的 SHA256；
- 仓库是否位于 `/mnt/*`。

`/mnt/*` 只产生迁移建议警告；系统版本、工具链、脚本或平台 ZIP 问题会返回非零退出码。

## 6. Bootstrap 与构建

```bash
./scripts/bootstrap.sh
./scripts/build.sh --client official
./scripts/build.sh --client iron
```

`bootstrap.sh` 校验官方归档：

```text
archive/legacy-2025/代码/Planner-release-2025(1).zip
SHA256 fdb9cf54054aaac0ccbfbeabc97a99d0067dc975f0999a90f81fe83a326ac150
```

解压源位于 `.work/planner-2025/`，official 和 Iron 分别使用独立工作副本。环境版本记录写入：

```text
.work/environment/official.txt
.work/environment/iron.txt
```

## 7. 运行和结果

official 基线：

```bash
./scripts/run_case.sh --stage 1 --mode it --case 01 --client official
```

Iron 四模式：

```bash
./scripts/run_suite.sh --manifest tests/manifests/smoke.csv
```

每次运行创建：

```text
artifacts/runs/<UTC时间>-s<stage>-<mode>-<case>-<client>-<pid>/
├─ client.log
├─ server.log
├─ platform-log/
└─ summary.json
```

成功标准不是仅看退出码，而是四种模式都出现实际动作、`Time` 和 `Score`，且 `summary.json` 中 `raw_score`、`official_score`、动作数和超时状态合理。

## 8. 全员 onboarding

```bash
./scripts/setup-ubuntu18.sh
./scripts/doctor.sh
./scripts/onboard.sh
```

`onboard.sh` 依次执行平台 bootstrap、official 构建和 Stage1 IT、Iron 构建及四模式 smoke。完成后复制 `docs/onboarding/member-template.md`，记录五次运行的 summary 路径、退出码和故障处理。

## 9. 题库校验

```bash
python3 -m unittest tools.test_validate_questions -v
python3 tools/validate_questions.py tests/problems
```

提交候选严格校验：

```bash
python3 tools/validate_questions.py tests/problems \
  --expected-per-stage 30 \
  --review-manifest tests/manifests/question-review.csv \
  --require-review \
  --json artifacts/question-validation.json
```

自动检查不能代替 IT/NT 语义等价、约束、可执行性和人工复核。

## 10. 常见故障

| 现象 | 处理 |
|---|---|
| `Ubuntu-18.04 is not installed` | 运行 Windows 安装脚本并完成首次账户初始化 |
| 环境被识别为 Ubuntu 26.04 等版本 | 使用 `wsl -d Ubuntu-18.04`，不要在默认发行版误跑 |
| `apt-get update` 的 Bionic 源缺失 | 核查原因后使用 `--repair-sources` |
| `/usr/bin/env: bash\r` | 确认 Git 属性；仅对受影响文件使用 `dos2unix`，不要全仓转换 |
| `.work/planner-2025 exists without the expected stamp` | 确认它只是失败缓存后移走，再重新 bootstrap |
| CMake 找不到 Boost | 重新运行 setup 和 doctor，确认没有混用其他发行版工具链 |
| cserver 一直等待 | 检查残留进程、客户端日志、模式、题号与 timeout |
| 分数为空 | 查看 server/platform log；`null` 不能解释为 0 分 |

## 11. 复现完成标准

- 至少两台机器从干净仓库完成 onboarding；
- official 和 Iron 均能构建；
- Iron 四模式各跑一题并产生完整 summary；
- `.work/environment/*.txt` 差异可解释；
- 固定 smoke 连续运行三次无新增崩溃、挂死或不可解释轨迹差异；
- 第二名队员无需口头指导即可完成。

参考资料：

- https://learn.microsoft.com/windows/wsl/install
- https://learn.microsoft.com/windows/wsl/install-manual
- https://learn.microsoft.com/windows/wsl/setup/environment
- https://archive.ubuntu.com/ubuntu/dists/bionic/Release
