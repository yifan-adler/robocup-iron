# 12 道 Stage 2 优势候选题

## 目标

这 12 道题不是按“任务执行数量最多”设计，而是按官方计分规则追求稳定总分：

- 用一个低成本目标激活约束计分；
- 让 Iron 的约束反推、多 `goto` 聚合和风险过滤直接转化为分数；
- 避免重复任务、重复约束、歧义物体和不稳定的 `takeout` 自然语言路径；
- 保留 IT/NT 一一对应，并让两种模式产生相同的物理动作轨迹。

所有文件都是 Stage 2 题，位于 `tests/problems/stage2/01.xml` 至 `12.xml`。

## 题型与 Iron 优势

| 题号 | 设计机制 | Iron 的得分优势 |
|---|---|---|
| 01-04 | 错误位置 + `must near` + 对错误地点的 `must not near` | 用约束矛盾唯一反推出正确位置，只执行一次 Move 和一次 PickUp |
| 05-07 | 2/3/4 个位置缺失 + 10 个不同的 `goto` 目标共址 | 约束补全缺失位置，多 `goto` 聚合后以 1 次 Move 为主完成目标组 |
| 08-09 | 1 个安全拾取目标 + 3 个高风险拾取目标 + 18 条不同约束 | 风险计算跳过会破坏多条约束的任务，保留约束分 |
| 10-11 | 1 个安全拾取目标 + 3 个受约束的容器任务 + 19 条不同约束 | 识别任务/约束冲突，不进行低收益容器动作和远距离移动 |
| 12 | 4 个位置缺失 + 8 个不同的共址 `goto` + 11 条约束 | 同时利用位置补全和多 `goto` 聚合 |

## 最终版本实测

每题使用仓库内官方 `cserver` 和 Iron 客户端执行 3 次 IT、3 次 NT，共 72 次最终版本运行。表中为官方计分；动作数在所有重复运行中保持一致。

| 题号 | IT 分数范围 | NT 分数范围 | 固定动作数 | 主要动作 |
|---:|---:|---:|---:|---|
| 01 | 430-436 | 434-438 | 2 | Move 1 + PickUp 1 |
| 02 | 434-436 | 434-436 | 2 | Move 1 + PickUp 1 |
| 03 | 434-438 | 432-436 | 2 | Move 1 + PickUp 1 |
| 04 | 436-438 | 432-436 | 2 | Move 1 + PickUp 1 |
| 05 | 636-648 | 638-642 | 5 | Move 1 + PickUp 2 + PutDown 2 |
| 06 | 608-620 | 602-622 | 7 | Move 1 + PickUp 3 + PutDown 3 |
| 07 | 580-604 | 580-592 | 9 | Move 1 + PickUp 4 + PutDown 4 |
| 08 | 460-476 | 470-480 | 2 | Move 1 + PickUp 1 |
| 09 | 468-478 | 436-474 | 2 | Move 1 + PickUp 1 |
| 10 | 488-498 | 490-498 | 2 | Move 1 + PickUp 1 |
| 11 | 486-500 | 414-500 | 2 | Move 1 + PickUp 1 |
| 12 | 520-546 | 524-548 | 9 | Move 1 + PickUp 4 + PutDown 4 |

分数波动来自平台效率分和宿主调度；最终版本 72 次运行中没有超时、崩溃、负分或动作轨迹变化。10、11 题即使效率奖励降到零，依靠 1 个目标和 19 条约束仍有 414 分基础保底。

## 已执行验证

```bash
python tools/validate_questions.py tests/problems --stage 2
python -m unittest tools.test_validate_questions -v
wsl -d Ubuntu-18.04 -- bash -lc \
  'cd /mnt/e/robot/robocup-iron && ./scripts/run_suite.sh --manifest tests/manifests/advantage-12.csv'
```

结果：

- XML、Stage 开关、对象 ID、颜色、任务/约束重复、IT/NL 数量和句号：0 error，0 warning；
- 校验器单元测试：3/3 通过；
- 官方平台最终版本：72/72 成功，0 timeout，0 client failure；
- IT 和 NT 均能解析并得到有效官方分数；
- 每题的实际状态满足所有 `must`/`must not` 信息约束，任务和约束条件中的对象描述唯一。

## 提交前仍需完成

自动校验和作者自查不能代替非作者复核。`tests/manifests/question-review.csv` 中 12 题保持 `pending`，需要一名队员逐题确认 IT/NT 语义、初始约束和赛事命名，然后填写 reviewer 并改为 `approved`。在此之前不能放入正式 30 题提交包。
