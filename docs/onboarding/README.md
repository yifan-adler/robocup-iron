# 全员环境验收记录

角色分配前，四名队员都要独立完成同一套验收。复制 `member-template.md` 为自己的记录，文件名可使用姓名拼音或约定代号。

完成标准：

1. `docker version` 与 `docker compose version` 正常；
2. `docker compose build` 成功；
3. `docker compose run --rm dev ./scripts/onboard.sh` 成功；
4. 能指出最新五个 `artifacts/runs/*/summary.json`；
5. 能解释 Stage1/Stage2 与 IT/NT 的参数差异；
6. 能从一次失败定位到宿主、容器、平台、客户端、题目或参数中的一层；
7. 将遇到的问题和解决方法写入个人记录。

不要提交 `artifacts/` 日志本身；只提交命令、结果摘要和必要错误片段。
