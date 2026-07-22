# Codex Workflow for Aurora
## Local Agent Development Contract

---

# 1. AGENTS.md 的定位

`AGENTS.md` 应当是索引和规则入口，不应变成完整百科全书。

它应告诉 Codex：

- 先读哪些文件；
- 如何构建；
- 如何测试；
- 哪些架构边界冻结；
- 哪些行为必须停止并询问；
- 完成时必须提供什么证据。

详细产品规格继续放在 `docs/specifications/` 和 `docs/handoff/`。

---

# 2. 每次 Codex Session 的启动顺序

让 Codex 先执行只读分析：

```text
1. Read AGENTS.md.
2. Read docs/handoff/00_HANDOFF_README.md.
3. Read docs/handoff/02_ARCHITECTURE_GUARDRAILS.md.
4. Read the task specification.
5. Inspect relevant code and tests.
6. Report current understanding and planned file changes.
7. Do not edit until the plan is accepted.
```

对很小的修复可以省略人工批准，但仍需明确目标、文件和验收标准。

---

# 3. 一个任务只解决一个问题

合格任务：

- 实现 Library Scanner；
- 修复 Transition 最终一帧跳变；
- 增加 Queue Persistence；
- 建立 MPRIS Adapter；
- 增加 Reduced Motion 测试。

不合格任务：

> 把 Aurora 做完整、优化 UI、加入 AI、接网易云并修好所有动画。

大任务必须先拆成可独立验证的子任务。

---

# 4. Codex Task Spec

每个任务建立：

```text
docs/tasks/AUR-XXX_<NAME>.md
```

内容：

```markdown
# Task
明确的一句话目标

## Context
为什么要做

## Source of Truth
需要读取的规范

## In Scope
允许实现的内容

## Out of Scope
明确不做什么

## Allowed Files
预计修改文件

## Architecture Constraints
不可违反的边界

## Acceptance Criteria
可验证结果

## Required Tests
必须执行的命令

## Visual Evidence
截图/视频要求

## Rollback
失败如何回退
```

---

# 5. 分支策略

推荐：

```text
main
└── feature/aur-xxx-short-name
```

示例：

```bash
git switch -c feature/aur-021-library-scanner
```

一个 Task 一个分支。

不要让 Codex 在未提交的大量本地修改上继续跨任务工作。

---

# 6. Codex 完成报告必须包含

```text
Summary
Changed Files
Architecture Impact
Commands Executed
Test Results
Visual Evidence
Known Limitations
Unresolved Questions
Suggested Next Task
```

“代码已完成”但没有构建和测试证据，不接受。

---

# 7. Review Session 与 Implementation Session 分开

建议使用两个独立上下文：

## Implementation Session

负责实现。

## Review Session

只读取：

- Task Spec；
- diff；
- 测试；
- 日志；
- 截图/视频。

要求 Review Session 尝试发现：

- 架构越界；
- 缺失测试；
- 状态不一致；
- 性能风险；
- Accessibility 回归；
- 数据丢失；
- 错误降级；
- 文档过期。

不要让同一个上下文既写代码又作为唯一审核者。

---

# 8. Motion 任务的特殊规则

Motion 必须：

- 使用 Release 构建验收；
- 记录显示器刷新率；
- 录制原始视频；
- 检查启动、主运动、减速、落点、图层交接；
- 验证 60 Hz；
- 有条件时验证 120 Hz；
- 验证 Reduced Motion；
- 检查重复点击；
- 检查窗口缩放；
- 检查场景动画和 Transition 并发。

Codex 不得仅凭“动画时长更长”宣称更流畅。

---

# 9. 数据任务的特殊规则

任何数据库或持久化任务必须提供：

- Schema；
- Version；
- Migration；
- Rollback；
- Corruption Handling；
- Delete；
- Export；
- Provenance；
- Fixture；
- Round-trip Test。

---

# 10. 建议审批模式

在不熟悉的任务上使用需要人工确认的模式。

在以下操作前必须人工确认：

- 删除文件；
- 修改 Schema；
- 新依赖；
- 网络访问；
- 系统包安装；
- 大规模重构；
- Git history 重写；
- Provider 认证；
- 用户数据迁移。

---

# 11. 文档同步

每个完成任务至少检查：

- `CHANGELOG.md`
- `README.md`
- 对应 `docs/reviews/`
- 对应 ADR
- Current Status
- Tests / Snapshot Index

代码行为改变但文档未更新，不算完整完成。
