# 阶段一最终 20 题验算报告

验算日期：2026-09-06（Asia/Shanghai）

## 结论

最终题集为 tests/problems/stage1/01.xml 至 20.xml。原 02、03 与 01 的结构化指令和自然语言完全重复，已分别替换为从 stage2/03.xml 和 stage2/29.xml 做真值归一化得到的新题。

- 20 个结构化指令集合全部唯一。
- 20 个自然语言集合全部唯一。
- Stage 1 的 mis、err/r、err/w 均为空。
- 静态校验：20 文件，0 error，0 warning。
- Ubuntu 18.04 校验器测试：5/5 通过。
- 官方客户端基线：IT/NT 共 40/40 次完成，0 超时。
- Iron：两次完整复跑共 80/80 次完成，0 超时。
- 40 个题目与模式组合中，Iron 的两次运行取最低分后仍全部高于官方基线。
- 最小单次优势为 314 分，最大单次优势为 590 分。

## 最终题单与配对成绩

Iron 分数取两次完整复跑中的较低值；优势取 IT 和 NT 中更小的一项。

| 题号 | 来源 | 主要优势结构 | 官方 IT | Iron IT 最低 | 官方 NT | Iron NT 最低 | 最小优势 |
|---|---|---|---:|---:|---:|---:|---:|
| 01 | stage2/05 | 多目标 goto 聚合 | 98 | 688 | 98 | 688 | +590 |
| 02 | stage2/03 | 安全 pickup 与关系约束 | 98 | 416 | 98 | 426 | +318 |
| 03 | stage2/29 | 安全 pickup 与关系约束 | 98 | 436 | 98 | 430 | +332 |
| 04 | stage2/12 | 多目标 goto 聚合 | 98 | 626 | 98 | 626 | +528 |
| 05 | stage2/10 | 冲突任务筛除与安全 pickup | 98 | 500 | 100 | 502 | +402 |
| 06 | stage2/11 | 冲突任务筛除与安全 pickup | 98 | 502 | 98 | 498 | +400 |
| 07 | stage2/09 | 冲突任务筛除与安全 pickup | 100 | 482 | 98 | 480 | +382 |
| 08 | stage2/08 | 冲突任务筛除与安全 pickup | 98 | 480 | 98 | 476 | +378 |
| 09 | stage2/01 | 安全 pickup 与关系约束 | 100 | 436 | 98 | 438 | +336 |
| 10 | stage2/04 | 安全 pickup 与关系约束 | 98 | 436 | 98 | 438 | +338 |
| 11 | stage2/26 | 安全 pickup 与关系约束 | 98 | 438 | 98 | 436 | +338 |
| 12 | stage2/16 | 安全 pickup 与关系约束 | 100 | 440 | 100 | 436 | +336 |
| 13 | stage2/21 | 安全 pickup 与关系约束 | 98 | 436 | 100 | 440 | +338 |
| 14 | stage2/22 | 安全 pickup 与关系约束 | 98 | 438 | 98 | 438 | +340 |
| 15 | stage2/24 | 安全 pickup 与关系约束 | 100 | 434 | 98 | 436 | +334 |
| 16 | stage2/27 | 安全 pickup 与关系约束 | 98 | 412 | 98 | 436 | +314 |
| 17 | stage2/30 | 安全 pickup 与关系约束 | 100 | 420 | 98 | 434 | +320 |
| 18 | stage2/02 | 安全 pickup 与关系约束 | 100 | 438 | 98 | 436 | +338 |
| 19 | stage2/23 | 安全 pickup 与关系约束 | 98 | 440 | 98 | 434 | +336 |
| 20 | stage2/19 | 安全 pickup 与关系约束 | 98 | 438 | 98 | 432 | +334 |

官方基线一轮总分为 3938。Iron 两轮总分分别为 18824 和 18782。两轮相同题目与模式的最大分数波动为 34 分，平均绝对波动为 5.35 分；波动主要来自运行时间计分，不改变任何一项胜负结论。

## 可复现入口

- Iron 清单：tests/manifests/selected-20-stage1.csv
- 官方基线清单：tests/manifests/selected-20-stage1-official.csv
- 人工复核表：tests/manifests/question-review.csv
- 静态报告：artifacts/stage1-20-static-validation-wsl.json
- 人工门禁报告：artifacts/stage1-20-review-gate.json
- 官方基线运行窗口：artifacts/runs/20260906T140449Z-* 至 20260906T140623Z-*
- Iron 第一轮：artifacts/runs/20260906T140633Z-* 至 20260906T140838Z-*
- Iron 第二轮：artifacts/runs/20260906T141006Z-* 至 20260906T141219Z-*

复跑命令：

    python3 -m unittest tools.test_validate_questions -v
    python3 tools/validate_questions.py tests/problems/stage1 --stage 1 --expected-per-stage 20 --require-unique-questions
    ./scripts/run_suite.sh --manifest tests/manifests/selected-20-stage1-official.csv
    ./scripts/run_suite.sh --manifest tests/manifests/selected-20-stage1.csv

## 提交前人工门禁

自动验算已经通过，但规则要求作者与复核者不能是同一人。队友仍需逐题检查初始状态、IT/NL 含义一致性和约束可执行性，并在 tests/manifests/question-review.csv 中填写 reviewer，将 status 从 pending 改为 approved。

完成后运行：

    python3 tools/validate_questions.py tests/problems/stage1 --stage 1 --expected-per-stage 20 --require-unique-questions --review-manifest tests/manifests/question-review.csv --require-review

当前门禁会有意报告 20 个 review.invalid；这表示尚未完成第二人签字，不是自动验算失败。

## 结论边界

这里的优越性是相对于仓库中的官方示例客户端，在当前 Ubuntu 18.04、当前平台构建和这 20 道题上的实测结果。它不能证明对所有未知题或其他参赛队都占优。比赛提交前仍必须完成第二人复核，并在比赛机上再跑一次冻结版本。
