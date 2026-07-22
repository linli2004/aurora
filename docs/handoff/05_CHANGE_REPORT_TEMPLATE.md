# Aurora Change Report Template
## 发给 ChatGPT 的开发汇报格式

复制下面模板，每完成一个任务或遇到一个重要问题时填写。

---

# 1. 基本信息

```text
Task ID:
Task Name:
Branch:
Commit:
Base Commit:
App Version:
Ubuntu:
Qt:
Build Type:
```

# 2. 目标

本次要解决什么问题？

```text
```

# 3. 实际修改

```text
Changed Files:
New Files:
Deleted Files:
Dependencies:
Schema Changes:
Public API Changes:
```

# 4. Codex 实现摘要

```text
Codex 采用了什么方案？
为什么选择该方案？
有哪些替代方案被放弃？
```

# 5. 构建与测试

```text
Configure Command:
Build Command:
Test Command:
Tests Passed:
Tests Failed:
Warnings:
```

附上关键终端输出。

# 6. 实机表现

```text
Expected:
Actual:
```

视觉任务附：

- Before 截图/视频；
- After 截图/视频；
- Release / Debug；
- 显示器刷新率；
- 是否 Reduced Motion；
- 是否 Eco / Balanced。

# 7. 已知问题

```text
```

# 8. 架构影响

```text
是否改变 Primitive 职责？
是否改变状态模型？
是否改变公共 QML API？
是否改变持久化？
是否改变隐私或 Provenance？
```

# 9. 需要 ChatGPT 决策

```text
1.
2.
3.
```

# 10. 下一步候选

```text
```

---

# ChatGPT 评审输出预期

ChatGPT 将返回：

```text
Status:
Architecture Review:
Product Review:
UX/Motion Review:
Regression Risks:
Required Follow-up:
Documentation Update:
Recommended Next Task:
Acceptance:
```
