# Aurora Current Project Status
## Handoff Baseline

> 本文件描述交接时的功能状态。实际代码版本必须以本地 Git HEAD 和运行结果为准。

---

# 1. 已验证能力

## 工程基础

- Qt 6 + QML + CMake 工程可配置、构建和启动；
- QML Module 正常加载；
- C++ Runtime 可向 QML 暴露类型化状态；
- 结构检查、Fixture 检查和 C++ 单元测试存在；
- Debug 与 Release 构建流程已建立。

## 音频

- 使用 `QMediaPlayer` 和 `QAudioOutput` 播放本地音频；
- 支持选择多个本地文件；
- 支持播放、暂停、上一首、下一首、Seek、音量；
- 支持播放结束后继续；
- 队列已实现循环语义；
- 支持部分本地元数据读取；
- 无封面时显示可信 Generated Identity；
- 能从解码音频 Buffer 得到 Level、Bass、Mid、High、Transient。

## 体验组件

- Aurora Core 原型；
- Aurora Moment 原型；
- Aurora Crystal 原型；
- Component Gallery；
- Aurora Home；
- Music Space；
- Home → Music Space Identity Transform；
- Field Scene；
- Flow Scene；
- Audio Diagnostics；
- Track Transition 原型。

---

# 2. 当前仍是原型的部分

## Transition

切歌 Transition 已经历多次迭代：

- 快速两阶段切换；
- 单时间线切换；
- Progressive Landing；
- Progressive Carousel 候选方案。

当前最终手感尚未冻结。

应将其标记为：

```text
Transition Motion Candidate
○ Product Acceptance Pending
○ Performance Validation Pending
○ Reduced Motion Validation Pending
```

不要把某个聊天中生成的版本直接当成最终 Motion Token。

## Crystal

- Generated Identity 可用；
- Embedded Artwork 路径存在；
- 真正复杂 Artwork 的 Recognition Safe Region 尚未系统验证；
- Identity Invariants 尚未完成大样本视觉评审。

## Moment

- 已有原型呈现；
- 尚未接入真实 Memory Repository；
- 尚未实现完整 Lifecycle、Meaning、Provenance 和 Persistence；
- 不能标记为 Production Primitive。

## Atmosphere

- 已可响应音频语义；
- Flow Scene 仍属于第一代视觉骨架；
- 尚未形成正式 Scene Plugin / Scene Contract；
- 尚未完成最终 Shader、GPU 分级和性能预算。

---

# 3. 尚未实现

- 本地音乐库扫描与索引；
- SQLite 持久化；
- 稳定 Track ID；
- 播放队列持久化；
- 收藏、最近播放和设置；
- MPRIS 系统媒体控制；
- Aurora Memory Engine；
- Moment Capture / Keep This Moment；
- Recall；
- Emotion Graph；
- Intelligence Layer；
- 在线 Music Provider；
- GNOME Presence；
- Desktop Atmosphere；
- 安装包、升级和数据迁移；
- 崩溃恢复和正式日志系统；
- 完整 Accessibility 和 Privacy Center。

---

# 4. 已知技术风险

1. QML 动画手感依赖 Release 构建、VSync、显示器刷新率和 GNOME 合成；
2. 复杂 QML 属性动画可能受 GUI 线程负载影响；
3. Audio Buffer 分析依赖 Qt Multimedia 后端能力；
4. 当前元数据索引是播放时解析，不适合大型音乐库；
5. 当前队列只在内存中存在；
6. 视觉参数仍可能散落在 QML 内部；
7. 现有 Changelog 中存在重复一级标题，需要整理；
8. Snapshot Harness 目录存在，但还没有形成完整自动视觉回归；
9. Transition 的音频切换时点、视觉交接和元数据更新仍需统一状态机；
10. 未确认用户本地仓库是否已应用最后一个候选补丁。

---

# 5. 交接时必须补充的环境记录

在本文件末尾追加：

```text
Ubuntu Version:
GNOME Version:
Qt Version:
Compiler:
GPU:
GPU Driver:
Display Refresh Rate:
Current Branch:
Current Commit:
Current App Version:
All Tests:
Release Build:
Current Accepted Transition:
Known Local Issues:
```

---

# 6. 状态结论

```text
Current Phase:
Executable Experience Prototype

Next Gate:
Local Development Baseline Freeze

Next Product Milestone:
Aurora Local Music Alpha
```
