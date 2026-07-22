# Aurora Decision and Risk Log

---

# Part A — ADR Template

文件名：

```text
docs/decisions/ADR-XXX-short-name.md
```

模板：

```markdown
# ADR-XXX — Title

## Status
Proposed / Accepted / Rejected / Superseded

## Date

## Context
为什么需要决策？

## Constraints
哪些产品、架构、隐私或性能边界不可违反？

## Options
### Option A
### Option B
### Option C

## Decision
最终选择什么？

## Consequences
正面影响、负面影响和维护成本。

## Validation
如何证明决策正确？

## Rollback
如何撤销？

## Related Tasks
```

---

# Part B — Active Risk Register

| ID | 风险 | 等级 | 触发信号 | 缓解措施 | Owner | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| R-001 | Transition 在 Release 仍有最终帧跳变 | High | 视频出现硬交接 | 统一目标几何与状态机 | User | Open |
| R-002 | QML 动画被 GUI 线程负载影响 | High | 音频/元数据更新时掉帧 | Animator / Render Thread / Profiler | User | Open |
| R-003 | 大型音乐库扫描阻塞 UI | High | 扫描时窗口卡顿 | Worker + incremental model | User | Future |
| R-004 | Metadata/Artwork 内存增长 | Medium | 多文件播放内存持续上涨 | Cache policy | User | Future |
| R-005 | Moment 与播放历史混淆 | High | 自动生成大量 Moment | Lifecycle + confidence + user control | Shared | Future |
| R-006 | AI 推断成为用户事实 | Critical | Suggested Meaning 直接展示 | Suggested/Confirmed separation | Shared | Guarded |
| R-007 | Provider 依赖非官方接口 | High | 接口频繁失效/条款风险 | Provider review gate | Shared | Future |
| R-008 | Shader 降级后身份丢失 | High | Eco 模式无法识别歌曲 | Recognition fallback | Shared | Open |
| R-009 | 文档与代码版本漂移 | Medium | README 与实际行为不一致 | 每任务文档检查 | User | Open |
| R-010 | 聊天版本号与本地 HEAD 混淆 | High | 不确定当前补丁 | Git HEAD 为唯一真相 | User | Mitigated |

---

# Part C — Open Decisions

交接时建议立即建立以下决策项：

1. 当前 Progressive Carousel 是否接受为默认 Transition？
2. Transition Engine 是 QML 状态机还是 C++ Timeline Controller？
3. Local Library 使用 SQLite 的最小 Schema 是什么？
4. Artwork Cache 使用文件缓存还是数据库 Blob？
5. Scene Framework 的公共语义输入是什么？
6. MPRIS 在 Local Music Alpha 前还是后？
7. Flatpak 与 `.deb` 的首个打包目标？
8. Qt 最低版本是否固定为 6.8？
