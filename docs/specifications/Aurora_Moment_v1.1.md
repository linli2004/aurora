# Aurora Design System v1.1

# Component Library

## 02 Aurora Moment v1.1 Production Refinement Specification

---

# 0. Document Status

## Component

Aurora Moment v1.1

## Classification

# Memory Primitive

## Current Status

```text
✓ Architecture Approved
✓ Memory Primitive Confirmed
✓ Object Model Frozen
✓ Design Review Passed
✓ Figma Architecture Ready
✓ Runtime Architecture Ready
○ QML Implementation Pending
○ Visual Snapshot Validation Pending
○ Production Validation Pending
```

Aurora Moment 当前属于：

# Specification Archive

尚未升级为：

# Production Primitive

只有在 QML 实现、状态映射、可访问性测试和视觉快照全部通过后，才可进入 Production Archive。

---

# 1. Primitive Responsibility Model

Aurora 当前三个核心 Primitive 的职责正式冻结。

| Primitive      | System Responsibility | Human Question |
| -------------- | --------------------- | -------------- |
| Aurora Core    | Runtime State         | 音乐现在是什么状态？     |
| Aurora Moment  | Memory Identity       | 为什么这一刻值得留下？    |
| Aurora Crystal | Music Identity        | 这是哪一个音乐存在？     |

严格边界：

```text
Aurora Core
≠ Music Identity

Aurora Moment
≠ Music Identity

Aurora Crystal
≠ Memory Identity
```

最终对应：

```text
Core
→ State

Moment
→ Memory

Crystal
→ Identity
```

---

# 2. Component Definition

Aurora Moment 不是：

* Card
* Song Tile
* Listening History
* Playlist Item
* Media Container

Aurora Moment 是：

> 保存音乐体验身份、时间环境与个人意义，并允许用户重新召回和进入的记忆原子。

英文正式定义：

> Aurora Moment is a memory primitive that preserves the identity, context, and human meaning of a musical experience, allowing it to be recalled and re-entered.

Aurora 可以保存和关联意义。

Aurora 不替用户创造人生意义。

---

# 3. System Position

Aurora Moment 位于 Memory System 和 Experience System 之间。

```text
Aurora Intelligence

        ↓

Memory Association Engine

        ↓

Aurora Moment

        ↓

Aurora Recall

        ↓

Music Space
```

Aurora Moment 负责：

* 保存一次音乐体验
* 绑定体验环境
* 表达人类时间
* 承载用户赋予的意义
* 建立记忆关联
* 提供 Recall 入口

Aurora Moment 不负责：

* 表达实时播放状态
* 保存音乐视觉身份
* 分析音频
* 生成情绪模型
* 渲染完整 Music Space

---

# 4. Moment Object Model

Aurora Moment 数据对象正式定义为：

```json
{
  "musicIdentity": {},
  "temporalIdentity": {},
  "emotionalIdentity": {},
  "contextIdentity": {},
  "meaningIdentity": {},
  "memoryIdentity": {},
  "memoryWeight": {},
  "provenance": {}
}
```

结构关系：

```text
Music Identity

+

Temporal Identity

+

Emotional Identity

+

Context Identity

+

Meaning Identity

+

Memory Identity

+

Provenance

=

Aurora Moment
```

---

# 5. Music Identity

Music Identity 保存形成该 Moment 的音乐对象引用。

```json
{
  "trackId": "",
  "title": "",
  "artist": "",
  "album": "",
  "artworkReference": "",
  "audioProfile": {},
  "spatialCharacter": ""
}
```

## Spatial Character

用于连接 Music Space。

允许值示例：

```text
Wide
Close
Floating
Dense
Open
Surrounding
```

Music Identity 只描述音乐是谁以及如何被空间感知。

它不承担 Moment 的个人意义。

Aurora Crystal 将负责 Music Identity 的视觉连续性。

---

# 6. Temporal Identity

Temporal Identity 必须同时保存：

* 机器可使用的准确时间
* 用户可感知的人类时间

```json
{
  "timestamp": "2026-07-21T23:40:00-07:00",
  "localTimezone": "America/Los_Angeles",
  "date": "2026-07-21",
  "season": "Summer",
  "periodLabel": "Late Summer Night"
}
```

## Machine Time

用于：

* 排序
* 同步
* 恢复
* 去重
* 数据迁移

## Human Time

用于：

* Moment 标题
* Recall
* Memory Flow
* 自然语言搜索

原则：

```text
Timestamp
→ Data Reliability

Period Label
→ Human Perception
```

---

# 7. Emotional Identity

Emotional Identity 来自 Aurora Emotion Graph。

```json
{
  "primary": "Nostalgia",
  "secondary": "Warmth",
  "intensity": 0.58,
  "evolution": [
    "Loneliness",
    "Acceptance",
    "Calm"
  ]
}
```

情绪不是固定标签。

它是具有时间变化的体验路径。

```text
Beginning

↓

Development

↓

Resolution
```

Moment 保存的是该次体验中的情绪状态，而不是歌曲永久固定的情绪定义。

---

# 8. Context Identity

Context Identity 遵循：

# Minimal Context

```json
{
  "device": "Laptop",
  "listeningPattern": "Repeated Listening",
  "environment": "Night Session",
  "externalContextEnabled": false
}
```

默认允许：

* 时间
* 设备状态
* 播放行为
* 本地音乐上下文

默认禁止：

* 精确位置
* 摄像头
* 麦克风
* 未经授权的外部数据

可选环境信息必须经过 Aurora Trust Layer。

---

# 9. Meaning Identity

Meaning Identity 保存人类赋予的意义。

```json
{
  "userNote": "First apartment",
  "personalTags": [
    "moving",
    "summer"
  ],
  "importance": 0.85,
  "userConfirmed": true
}
```

核心原则：

# Meaning Belongs to Human

AI 可以：

* 提示可能的关联
* 整理上下文
* 建议 Moment

AI 不可以：

* 自动声明人生意义
* 编造个人故事
* 将系统推断伪装成用户记忆

---

# 10. Memory Identity

Memory Identity 负责 Moment 与 Memory System 的关系。

```json
{
  "state": "Remembered",
  "confidence": 0.82,
  "relation": [
    "Summer Memory",
    "Night Listening"
  ],
  "ownership": "PersonalMemory",
  "preservationLevel": "Remembered"
}
```

## Ownership

允许值：

```text
SystemSuggested
UserAccepted
PersonalMemory
```

转换：

```text
System Suggested

↓

User Accepted

↓

Personal Memory
```

## Preservation Level

允许值：

```text
Temporary
Remembered
Important
Permanent
```

它描述保存关系，不直接决定视觉亮度。

---

# 11. Provenance

Provenance 用于 Trust Layer、Privacy Center 和 Memory Transparency。

```json
{
  "source": {
    "temporalIdentity": "System Context",
    "emotionalIdentity": "Local Emotion Model",
    "meaningIdentity": "User Created",
    "contextIdentity": "Local Device Context"
  },
  "generatedBy": "Aurora Local Runtime",
  "userConfirmed": true,
  "lastUpdated": "2026-07-21T23:55:00-07:00"
}
```

Aurora 必须能够回答：

```text
这个 Moment 使用了什么数据？

数据在哪里处理？

哪些内容由系统推断？

哪些内容由用户创建？

它是否可以被修改或删除？
```

Provenance 默认不显示在 Moment 表面。

它通过：

* Privacy Center
* Moment Transparency
* Memory Detail

按需提供。

---

# 12. Memory Confidence System

Memory Confidence 是 Memory Association Engine 的内部参数。

范围：

```text
0.0 → 1.0
```

示例：

```text
First Playback
0.20

Repeated Playback
0.55

Long Session
0.68

User Accepted
0.90

Keep This Moment
1.00
```

Memory Confidence 决定 Moment 是否有资格进入：

```text
Candidate

↓

Observed

↓

Remembered
```

用户日常界面不显示数值。

---

# 13. Memory Weight System

```json
{
  "intensity": 0.42,
  "depth": 0.78,
  "personalMeaning": 0.92
}
```

字段职责：

## Intensity

当时的情绪变化强度。

## Depth

Moment 随时间和重复关联形成的关系深度。

## Personal Meaning

用户主动赋予的重要程度。

最终视觉映射：

```text
Memory Weight

↓

Semantic Memory Presence

↓

Material Depth

Temporal Trace

Spatial Weight
```

重要规则：

> Personal Meaning 不得直接映射为更高亮度。

高意义、低强度 Moment 可以表现为：

```text
Low Brightness

Deep Temporal Layer

Stable Memory Trace

Low Ambient Motion
```

意义通过重量表达，而不是通过炫目表达。

---

# 14. Memory Lifecycle

Memory Lifecycle 由 Memory Engine 管理。

```text
Created

↓

Candidate

↓

Observed

↓

Remembered

↓

Meaningful

↓

Recalled

↓

Archived
```

## Created

一次音乐体验已经发生。

它还不是记忆。

## Candidate

系统发现该体验可能值得保留。

## Observed

系统检测到持续关系或用户关注。

## Remembered

Moment 正式进入用户记忆体系。

## Meaningful

用户主动增加意义、标签或说明。

## Recalled

Moment 被用户重新召回。

## Archived

Moment 仍属于用户，但退出日常主动展示。

---

# 15. Experience State

组件视觉层不直接暴露完整 Memory Lifecycle。

正式 Experience State：

```text
Dormant

Present

Remembered

Meaningful

Recalling

Archived
```

映射关系：

| Memory Lifecycle | Experience State  |
| ---------------- | ----------------- |
| Created          | Dormant           |
| Candidate        | Dormant / Present |
| Observed         | Present           |
| Remembered       | Remembered        |
| Meaningful       | Meaningful        |
| Recalled         | Recalling         |
| Archived         | Archived          |

原则：

```text
Lifecycle
→ System Meaning

Experience State
→ Human Perception
```

---

# 16. Runtime State

Runtime State 与 Experience State 独立。

```text
Unbound
Binding
Rendering
Restoring
Ready
Failure
Sleeping
```

组件状态模型：

```text
Memory Lifecycle

+

Experience State

+

Runtime State

+

Lifecycle Renderer
```

错误状态不应伪装为记忆状态。

例如：

```text
Restoring
≠ Recalling
```

Restoring 是系统行为。

Recalling 是用户体验。

---

# 17. Archived Experience State

Archived 不代表删除。

正式定义：

> 记忆仍然属于用户，但不再主动进入日常体验。

视觉行为：

```text
Ambient Motion:
Off

Memory Trace:
Retained

Presence:
Low

Recall:
Available

Home Visibility:
Hidden by Default
```

Archived Moment 可出现在：

* Aurora Recall Search
* 完整 Memory Archive
* 用户指定集合

它默认不出现在：

* Current Moment
* Home Memory Flow
* 主动推荐

---

# 18. Geometry System

Aurora Moment 使用：

# Soft Organic Square

比例：

```text
1 : 1
```

尺寸：

| Size    |   Dimensions | Context           |
| ------- | -----------: | ----------------- |
| Compact | 240 × 240 px | Memory Flow       |
| Primary | 400 × 400 px | Home              |
| Recall  | 640 × 640 px | Recall Experience |

圆角规则：

```text
Radius
=
Short Edge × 16%
```

示例：

```text
240px → approximately 38px

400px → 64px

640px → approximately 102px
```

基础几何保持稳定。

表达变化来自：

* Atmosphere
* Memory Material
* Optical Weight
* Temporal Trace

---

# 19. Optical Architecture

Aurora Moment 使用 Foundation Optical Token。

```json
{
  "centerBalance": "CenterWeighted",
  "lightWeight": "MemoryDependent",
  "depthBias": "Mid",
  "visualGravity": "AtmosphereCenter"
}
```

Moment 的视觉中心不是文本区域。

视觉中心必须位于：

# Atmosphere Safe Zone

文字不得破坏中心呼吸空间。

---

# 20. Moment Layer Architecture

```text
Aurora Moment


Layer 01
Surface Identity


Layer 02
Atmosphere Field


Layer 03
Music Identity Reference


Layer 04
Temporal Trace


Layer 05
Memory Material


Layer 06
Meaning Indicator
```

## Surface Identity

提供基础边界与信息可读性。

## Atmosphere Field

表达该次体验的情绪环境。

## Music Identity Reference

由 Aurora Crystal 或简化 Artwork Reference 表达音乐身份。

## Temporal Trace

表达人类时间与记忆痕迹。

## Memory Material

表达保存深度。

## Meaning Indicator

仅在用户主动赋予意义时出现。

---

# 21. Material Dependency

Aurora Moment 依赖：

```text
Aurora Glass

+

Atmosphere Material

+

Memory Material
```

Memory Material 包含：

```text
Soft Refraction

Temporal Trace

Depth Persistence

Low-Frequency Light Memory
```

设计目标：

```text
Frozen Moment
```

而不是：

```text
Fantasy Crystal
```

Aurora Moment 不得呈现为：

* 宝石
* 奖杯
* 游戏收藏品
* 稀有度卡片

---

# 22. Interaction Model

Aurora Moment 继承：

# State Before Action

和：

# Observe Before Interact

交互顺序：

```text
Observe

↓

Approach

↓

Interact

↓

Recall or Enter
```

## Hover

```text
Translate Y:
-4px

Scale:
1.015

Atmosphere Presence:
Slight Increase
```

Hover 表示靠近，不表示浮起。

## Click

进入与该 Moment 关联的 Music Space。

## Long Press

触发：

# Recall Moment

## Keep This Moment

用户主动将候选 Moment 提升为个人记忆。

不得使用：

```text
Save Track
```

因为保存歌曲和保存经历是不同操作。

---

# 23. Recall Relationship

```text
Aurora Moment

↓

Aurora Recall

↓

Context Restoration

↓

Music Space
```

Recall 恢复：

* 时间氛围
* 光线温度
* Motion Personality
* Memory Trace
* Music Identity
* 用户意义

Recall 不只是重新播放同一首歌。

它是恢复该次音乐体验的可感知环境。

---

# 24. Moment → Music Space Transform

原链路废弃：

```text
Moment

↓

Core

↓

Music Space
```

正式链路：

```text
Aurora Moment

↓

Moment Boundary Dissolves

↓

Aurora Crystal Emerges

↓

Aurora Core Awakens

↓

Atmosphere Expansion

↓

Music Space
```

职责：

## Aurora Moment

提供记忆与上下文。

## Aurora Crystal

保持音乐视觉身份连续。

## Aurora Core

表达进入、恢复和播放状态。

## Atmosphere

展开情绪和环境空间。

用户感知：

```text
记忆对象被打开

↓

音乐身份持续存在

↓

音乐状态苏醒

↓

体验空间展开
```

这是 Aurora Identity Preservation 的正式实现。

---

# 25. Figma Architecture

## Variants

```text
Size

Experience State

Context

Accessibility
```

### Size

```text
Compact
Primary
Recall
```

### Experience State

```text
Dormant
Present
Remembered
Meaningful
Recalling
Archived
```

### Context

```text
Home
Memory Flow
Recall
Archive
```

### Accessibility

```text
Normal
Reduced Motion
High Contrast
Minimal
```

## Component Properties

```text
Has Meaning
Has User Note
Recall Enabled
Ownership
Memory Presence
Atmosphere Binding
```

## Variable Bindings

```text
Emotion Color

Memory Depth

Temporal Trace

Ambient Attention

Optical Gravity
```

Candidate、Observed 等内部 Lifecycle 状态不建立独立视觉 Variant。

---

# 26. QML Data Binding

数据对象与呈现对象必须分离。

```qml
AuroraMoment {
    moment: AuroraMomentData {
        musicIdentity: musicData
        temporalIdentity: temporalData
        emotionalIdentity: emotionData
        contextIdentity: contextData
        meaningIdentity: meaningData
        memoryIdentity: memoryData
        memoryWeight: weightData
        provenance: provenanceData
    }

    size: AuroraMoment.Primary
    context: AuroraMoment.Home
    accessibilityMode: AuroraAccessibility.Normal
    recallEnabled: true
}
```

原则：

```text
Moment Data
≠ Moment Presentation
```

Memory System 提供对象数据。

AuroraMoment 组件只声明：

* 呈现尺寸
* 使用场景
* 可访问性模式
* 是否允许 Recall

禁止组件调用方直接设置：

```qml
glowColor
blurAmount
shadowOpacity
memoryTraceDepth
```

这些由 Token Resolver 计算。

---

# 27. Runtime Pipeline

```text
Aurora Moment Data

↓

Lifecycle Resolver

↓

Experience State Mapper

↓

Trust / Provenance Resolver

↓

Semantic Token Resolver

↓

Material System

↓

Runtime Renderer

↓

Aurora Moment
```

Moment 不直接读取底层模型原始数据。

所有视觉参数必须经过语义解析。

---

# 28. Production Validation

Aurora Moment 进入 Production 前必须通过：

## Identity Test

用户能否感受到这是一次具体体验，而不是歌曲卡片？

## Memory Test

组件能否区别播放记录和个人记忆？

## State Mapping Test

```text
Lifecycle State

↓

Experience State

↓

Visual Output
```

是否保持正确映射？

## Transform Test

Moment、Crystal、Core 和 Atmosphere 是否保持连续身份？

## Localization Test

以下内容在不同语言中是否稳定：

* Period Label
* User Note
* Personal Tag
* Date Format
* Artist and Song Title

## Privacy Provenance Test

自动形成的 Moment 是否能解释：

* 使用了什么数据
* 在哪里处理
* 是否经过用户确认
* 是否可以修改或删除

## Accessibility Test

必须覆盖：

* Reduced Motion
* High Contrast
* Keyboard Only
* Cognitive Minimal
* Text Scale 150%

## Performance Test

必须覆盖：

* Integrated GPU
* Low Power Mode
* Archive Large Dataset
* Recall Restore

## Visual Snapshot Test

最低参考集：

```text
AuroraMoment.Present.Home

AuroraMoment.Remembered.MemoryFlow

AuroraMoment.Meaningful.Recall

AuroraMoment.Recalling.Recall

AuroraMoment.Archived.Archive
```

---

# 29. Final Architecture

```text
Aurora Moment


Memory Primitive


├ Moment Data Object

│
├ Music Identity
├ Temporal Identity
├ Emotional Identity
├ Context Identity
├ Meaning Identity
├ Memory Identity
├ Memory Weight
└ Provenance


├ Memory Lifecycle


├ Experience State


├ Runtime State


├ Atmosphere Binding


├ Memory Material


├ Recall Interaction


├ Identity-Preserving Transform


└ Runtime Renderer
```

---

# 30. Final Definition

> Aurora Moment is a memory primitive that preserves the identity, context, and human meaning of a musical experience, allowing it to be recalled and re-entered.

中文：

> Aurora Moment 是保存音乐体验身份、环境与个人意义，并允许用户重新召回和进入的记忆原子。

---

# 31. Component Library Status

```text
Aurora Foundation

✓ Production Foundation


Aurora Core

✓ Production Primitive


Aurora Moment v1.1

✓ Specification Approved
✓ Production Refinement Complete
○ QML Implementation Pending
○ Visual Snapshot Validation Pending
○ Production Validation Pending


Next:

Aurora Crystal v1.0
```

---

# 32. Aurora Crystal Boundary

Aurora Crystal 正式分类为：

# Music Identity Primitive

三个 Primitive 的关系冻结为：

```text
Aurora Moment
=
Memory Identity


Aurora Crystal
=
Music Identity


Aurora Core
=
Runtime State
```

Music Space 组合：

```text
Moment supplies memory

Crystal preserves identity

Core expresses state

Atmosphere creates environment
```

Aurora Crystal 的核心任务：

> 定义音乐身份在不同 Moment、不同状态和不同空间中，如何始终被用户认出。

下一阶段正式进入：

# 03 Aurora Crystal v1.0 Specification

