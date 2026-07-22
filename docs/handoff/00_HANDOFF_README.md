# Aurora Music Framework
## Local Development Handoff v1.0

**交接日期：** 2026-07-22  
**项目阶段：** Initial Development / Executable Half-Finished Demo  
**开发方式：** 用户本地开发 + Codex 编码辅助 + ChatGPT 产品/架构评审辅助

---

# 1. 交接目的

本交接包用于把 Aurora 从“由聊天中持续生成补丁的原型项目”，转换为：

> 一个在用户本地 Git 仓库中，以 Codex 为编码代理、以规范和测试为约束、由用户最终决策的持续开发项目。

从本次交接开始：

- 用户负责本地仓库、分支、构建、运行、合并和最终产品决策；
- Codex 负责读取仓库、实现任务、运行命令、补充测试和提交代码建议；
- ChatGPT 不再直接生成项目代码或补丁；
- ChatGPT 继续负责产品方向、架构边界、设计一致性、需求拆解、问题诊断、验收标准、风险分析和文档维护。

---

# 2. 当前项目性质

Aurora 当前不是 Production 应用。

它是一个已经证明关键架构可运行的半成品 Demo，具备：

- Qt 6 / QML / CMake 原生 Ubuntu 应用；
- 本地音频播放；
- 本地元数据和可信 Generated Identity；
- Aurora Core、Moment、Crystal 原型；
- Home → Music Space 连续体验；
- Audio-Reactive Atmosphere；
- Flow / Field 场景；
- 本地多曲队列与循环；
- 切歌 Transition 原型；
- 基础单元测试和结构验证。

当前状态应表述为：

```text
Architecture Proven in Running Code
≠
Production Ready
```

---

# 3. 本地仓库是唯一代码真相

从现在起，聊天中曾经提供的压缩包和补丁不再是代码真相。

唯一代码真相是：

```text
用户本地 Git 仓库当前 HEAD
```

首次交接时必须记录：

```bash
git status --short
git rev-parse --short HEAD
git log -1 --oneline
```

如果项目还没有 Git 历史：

```bash
git init
git add .
git commit -m "chore: freeze Aurora local handoff baseline"
git tag handoff-baseline
```

如果已有 Git 历史：

```bash
git tag handoff-baseline-$(date +%Y%m%d)
```

不要根据窗口标题或聊天版本号推断真实代码状态。

---

# 4. 建议放置位置

将本交接包复制到仓库：

```text
docs/handoff/
```

推荐结构：

```text
docs/handoff/
├── 00_HANDOFF_README.md
├── 01_CURRENT_PROJECT_STATUS.md
├── 02_ARCHITECTURE_GUARDRAILS.md
├── 03_LOCAL_DEVELOPMENT_ROADMAP.md
├── 04_CODEX_WORKFLOW.md
├── 05_CHANGE_REPORT_TEMPLATE.md
├── 06_DECISION_AND_RISK_LOG.md
├── 07_ACCEPTANCE_CHECKLIST.md
├── 08_NEXT_TASKS.md
└── CODEX_START_PROMPT.md
```

---

# 5. 每次开发的最小闭环

```text
明确目标
↓
建立 Task Spec
↓
Codex 给出计划
↓
Codex 修改代码
↓
构建与测试
↓
Release 实机验证
↓
用户评审体验
↓
向 ChatGPT 汇报
↓
架构/产品评审
↓
合并与更新文档
```

没有构建、测试或真实运行证据的任务，不得标为完成。

---

# 6. 交接后的沟通规则

用户向 ChatGPT 汇报时，尽量提供：

- 当前 commit / branch；
- 本次目标；
- 修改文件；
- 测试结果；
- 截图或视频；
- 实际体验问题；
- Codex 的实现说明；
- 需要决策的问题。

ChatGPT 将返回：

- 当前阶段判断；
- 是否违反冻结边界；
- 产品和交互评审；
- 回归风险；
- 下一任务建议；
- 是否需要更新规范或 ADR；
- 验收是否通过。

---

# 7. 首个本地动作

先不要继续增加新功能。

首先完成：

1. 冻结本地 Git 基线；
2. 确认全部测试通过；
3. 使用 Release 构建；
4. 记录 Qt、Ubuntu、GPU 和显示器刷新率；
5. 验证当前 Carousel Transition；
6. 决定保留、继续调整或回退该 Transition；
7. 再进入 Local Library Alpha。
