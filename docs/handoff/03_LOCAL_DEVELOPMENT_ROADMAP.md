# Aurora Local Development Roadmap
## From Demo to Local-First Alpha

---

# Milestone 0 — Baseline Freeze

目标：把聊天生成原型转换成可维护本地仓库。

交付：

- Git 基线和 tag；
- 全部测试通过；
- Release 构建通过；
- 当前功能录像；
- 环境信息；
- Accepted / Rejected Transition 结论；
- Known Issues；
- 文档包进入仓库。

完成条件：

```text
Local HEAD Frozen
+
Tests Green
+
Release Runs
+
Current UX Recorded
```

---

# Milestone 1 — Local Music Alpha

目标：从“选择几个文件播放”升级为“可日常使用的本地播放器基础”。

功能：

- 音乐目录选择；
- 递归扫描；
- 稳定 Track ID；
- Metadata Index；
- Artwork Cache；
- SQLite；
- Library Model；
- Queue Model；
- 最近播放；
- 播放状态恢复；
- 基础设置；
- 错误文件隔离。

暂不加入：

- AI；
- Memory；
- 在线 Provider；
- GNOME 深度集成。

推荐拆分：

```text
M1.1 Track Identity Model
M1.2 Library Scanner
M1.3 SQLite Repository
M1.4 Library UI
M1.5 Persistent Queue
M1.6 Settings
```

---

# Milestone 2 — Motion and Transition System

目标：把一次性 QML 动画升级为正式 Transition Engine。

要求：

- Transition State Machine；
- source / destination identity snapshot；
- audio switch point；
- metadata switch point；
- cancellation policy；
- repeated input policy；
- circular queue integration；
- 60 / 120 Hz 体验验证；
- Reduced Motion；
- Release profiling。

正式提供三种 Personality：

1. Progressive Carousel  
   当前候选：封面放大、弧线退出、渐隐；新封面渐现、渐大、稳定。

2. Liquid Flow  
   视觉环境融合，适合柔和连续体验。

3. Mechanical Pulse 或 Organic Morph  
   最终名称由设计评审决定，不绑定音乐流派。

用户选择的是 Transition Personality，不是 Genre Rule。

---

# Milestone 3 — Visual Scene Framework

目标：把 Flow Scene 从单文件效果升级为可扩展 Scene Runtime。

建立：

```text
Scene Interface
├ Identity Input
├ Audio Semantic Input
├ Atmosphere Input
├ Quality Input
├ Accessibility Input
└ Lifecycle
```

首批场景：

- Field；
- Flow；
- 第三场景候选：Memory Mist / Spatial Ribbon / Quiet Room。

要求：

- Scene 热切换不中断音频；
- Eco / Balanced / Immersive；
- GPU 预算；
- 中央 Identity Safe Region；
- Shader Failure Fallback；
- Diagnostics；
- Snapshot / Video Baseline。

---

# Milestone 4 — Ubuntu Player Integration

目标：成为真正的 Ubuntu 原生播放器。

功能：

- MPRIS；
- 媒体快捷键；
- 系统通知；
- 单实例；
- 文件打开关联；
- 后台播放；
- Session Restore；
- 基础日志；
- `.deb` 或 Flatpak 初步打包。

桌面融合暂时只做到 Level 0–1：

- Off；
- Color Hint。

---

# Milestone 5 — Aurora Memory Alpha

目标：让 Moment 从演示组件变成真实 Memory Primitive。

功能：

- Moment Capture；
- Candidate；
- Keep This Moment；
- Suggested / Confirmed Meaning；
- User Note；
- Personal Tags；
- Memory Timeline；
- Archive；
- Detached；
- Relink；
- Recall Prototype；
- Export Fixture；
- Provenance。

必须先完成数据模型与迁移测试，再做视觉。

---

# Milestone 6 — Intelligence Prototype

目标：让 Aurora 理解音乐，但不展示“AI 正在工作”。

输入：

- Audio Profile；
- Track Metadata；
- Lyrics（有合法来源时）；
- User-confirmed behavior；
- Context。

输出：

- Emotion Graph；
- Atmosphere Parameters；
- Transition Personality Suggestion；
- Moment Candidate Confidence。

禁止：

- 自动编造用户故事；
- 未确认意义写入永久记忆；
- AI 直接控制 QML 原始参数。

---

# Milestone 7 — Provider Architecture

目标：在不污染核心的前提下接入在线音乐源。

先建立接口：

```text
MusicProvider
├ authenticate()
├ search()
├ resolveTrack()
├ resolveStream()
├ artwork()
├ availability()
└ provenance()
```

先实现：

- LocalProvider；
- 一个合法、稳定、可维护的在线 Provider。

网易云或其他来源必须先评估：

- 官方 API；
- 用户授权；
- 服务条款；
- Stream 合法性；
- 缓存与 DRM；
- 地区限制；
- 维护风险。

---

# Milestone 8 — Production Refinement

目标：从 Alpha 进入 Production Candidate。

必须完成：

- Accessibility；
- Privacy Center；
- Migration；
- Crash Recovery；
- Large Library；
- Integrated GPU；
- Battery；
- 120 Hz / 60 Hz；
- High DPI；
- 多语言；
- Snapshot；
- 性能基准；
- 安装、升级、卸载；
- 数据删除和导出。

---

# 推荐优先级

```text
P0 Baseline Freeze
P0 Local Library
P0 Stable Playback
P1 Motion Engine
P1 Scene Framework
P1 MPRIS
P2 Memory
P2 Intelligence
P3 Provider
P3 Desktop Atmosphere
```

不要在 Local Library 和持久化未稳定时进入 Provider 或 AI。
