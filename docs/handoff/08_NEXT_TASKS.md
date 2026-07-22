# Aurora Next Tasks
## Recommended Local Backlog

---

# AUR-013 — Freeze Local Handoff Baseline

目标：

- 确认本地 HEAD；
- 全部测试；
- Release；
- 环境记录；
- 录制当前功能；
- 建立 tag。

验收：

```text
handoff-baseline tag exists
all tests green
release launches
current UX recorded
```

---

# AUR-014 — Transition Acceptance Review

不要立即重写。

先建立对比：

- 当前实现；
- 用户期望时间线；
- 60 Hz 视频；
- 120 Hz 视频（支持时）；
- QML Profiler；
- 动画阶段表。

输出：

- Accept；
- Refine；
- Revert；
- Re-architect。

若继续开发，先决定 Transition 是否需要独立 State Machine。

---

# AUR-015 — Repository Cleanup

- 整理重复 Changelog 标题；
- 更新 README 当前真实版本；
- 更新 Current Status；
- 检查未使用 QML；
- 检查 TODO；
- 检查未跟踪文件；
- 建立 `docs/tasks/`；
- 建立 Snapshot Index。

这是文档和结构任务，不改变产品行为。

---

# AUR-016 — Stable Track Identity

Status:

```text
Implemented in local development after AUR-013.
```

在音乐库之前冻结：

```text
TrackId
SourceId
FilePath
CanonicalTitle
Artist
Album
Duration
ArtworkRef
Availability
Provenance
```

要求：

- 文件重命名策略；
- 文件移动策略；
- 无 Metadata；
- 重复文件；
- Hash 成本；
- Unicode；
- 测试 Fixture。

---

# AUR-017 — SQLite Repository ADR

只做设计和 ADR：

- Schema；
- Migration；
- Index；
- Artwork Cache；
- Delete；
- Detached；
- Library size；
- Threading。

评审后再编码。

---

# AUR-018 — Local Library Scanner

依赖 AUR-016、AUR-017。

要求：

- 后台扫描；
- 增量结果；
- 取消；
- 进度；
- 错误隔离；
- 不阻塞 UI；
- 重扫；
- 删除检测。

---

# AUR-019 — Local Library UI

第一版应保持克制：

- 最近播放；
- Tracks；
- Albums；
- Search；
- 当前队列入口。

不要提前制作 Galaxy 或复杂 Memory Timeline。

---

# AUR-020 — Persistent Queue and Session Restore

- 队列保存；
- 当前索引；
- 当前位置；
- 音量；
- Scene；
- Transition Personality；
- 异常退出恢复。

---

# AUR-021 — MPRIS

- 系统媒体键；
- GNOME 媒体菜单；
- Track metadata；
- Play/Pause/Next/Previous；
- Position；
- 单实例策略。

---

# AUR-022 — Scene Runtime Contract

在增加第三个场景前冻结 Scene API。

---

# 建议顺序

```text
AUR-013
→
AUR-014
→
AUR-015
→
AUR-016
→
AUR-017
→
AUR-018
→
AUR-019
→
AUR-020
→
AUR-021
→
AUR-022
```
