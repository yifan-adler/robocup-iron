# 全员环境验收记录

角色分配前，四名队员都要在各自的 WSL2 Ubuntu 18.04 中完成同一套验收。复制 `member-template.md` 为自己的记录，文件名使用姓名拼音或团队约定代号。

完成标准：

1. Windows `install-wsl-ubuntu1804.ps1 -CheckOnly` 通过；
2. Ubuntu 中 `setup-ubuntu18.sh` 与 `doctor.sh` 返回 0；
3. 仓库位于 `/home/<用户>/...`，不是长期在 `/mnt/*` 构建；
4. `./scripts/onboard.sh` 返回 0；
5. 能指出最新五个 `artifacts/runs/*/summary.json`；
6. 能解释 Stage1/Stage2 与 IT/NT 参数差异；
7. 能从失败日志定位到 Windows/WSL、Ubuntu 工具链、平台、Iron、题目或参数中的一层；
8. 将命令、退出码、问题和解决方法写入个人记录。

不要提交 `artifacts/` 日志本身，只提交结果摘要和必要错误片段。
