# Aurora Design System v1.1

# Component Library Archive

## Aurora Core v1.1 Final Specification

---

# 0. Archive Status

## Component

Aurora Core v1.1

## Status

```
✓ Approved
✓ Production Ready
✓ Runtime Ready
✓ QML Ready
✓ Figma Ready
✓ Design System Primitive
✓ Archive Locked
```

---

# 1. Component Classification

## Aurora Core

正式定位：

# Experience Runtime Primitive

Aurora Core 不属于：

* Button
* Control
* Widget
* Player Control
* Visual Decoration

Aurora Core 是：

> Aurora 将系统理解转换为人类可感知体验的最小状态接口。

---

# 2. System Position

Aurora Core 在 Aurora 架构中的位置：

```
Aurora Intelligence

        ↓

Atmosphere Generator

        ↓

Aurora Core

        ↓

Human Perception
```

---

Aurora Core 负责：

* 状态表达
* 感知转换
* 体验入口

Aurora Core 不负责：

* 音频分析
* 情绪计算
* 数据存储
* 内容推荐

---

# 3. Aurora Component Constitution v1.0

正式成立。

未来所有 Aurora Component 必须通过三个验证：

---

## State Validation

组件表达什么状态？

---

## Meaning Validation

组件传递什么意义？

---

## Interaction Validation

用户为什么需要与它交互？

---

组件必须满足：

```
State

↓

Meaning

↓

Interaction
```

禁止：

* 装饰性组件
* 炫技组件
* AI 展示组件

---

# 4. Aurora Runtime Pattern v1.0

所有 Aurora 核心组件统一采用：

```
Experience Layer

        ↓

Runtime Layer

        ↓

Lifecycle Layer

        ↓

Render Layer
```

---

统一状态模型：

```
Experience State

+

Runtime State

+

Lifecycle

+

Token Resolver

+

Renderer
```

---

# 5. Foundation Token System v1.1

Aurora Foundation 正式升级：

```
01 Color

02 Typography

03 Material

04 Motion

05 Spatial

06 Accessibility

07 Semantic

08 Optical

09 Ambient Behavior
```

---

# 6. Optical Token

Aurora 新增视觉基础层。

传统 UI：

关注：

```
Position

Size

Alignment
```

Aurora：

关注：

```
Visual Gravity

Light Weight

Depth Balance
```

---

结构：

```json
{
    CenterBalance,
    LightWeight,
    DepthBias,
    VisualGravity
}
```

---

用途：

统一：

* Aurora Core
* Aurora Crystal
* Memory Node
* Atmosphere Seed

---

# 7. Ambient Behavior System

正式成立。

Aurora 区分：

## Motion

用于：

状态变化。

例如：

* Transition
* Interaction
* Transform

## Ambient Behavior

用于：

存在感。

例如：

* Idle Drift
* Desktop Atmosphere
* Background Presence

---

结构：

```json
{
    Speed,
    Intensity,
    Persistence,
    AttentionLevel
}
```

---

Attention Level：

```
0.0

↓

1.0
```

---

标准：

| Experience      | Attention |
| --------------- | --------: |
| Idle Drift      |       0.1 |
| Home Atmosphere |       0.2 |
| Playing         |       0.4 |
| Recall          |       0.7 |
| Transition      |       0.8 |

---

# 8. Aurora Visual Reference System v1.0

建立统一视觉标准。

目的：

连接：

```
Figma

↓

Qt/QML

↓

Shader

↓

Runtime
```

---

目录：

```
Aurora Reference


Foundation


├ Aurora Core

├ Idle

├ Playing Calm

├ Playing Energy

├ Recall

└ Transition


Components


├ Aurora Moment

├ Aurora Crystal

├ Aurora Presence

├ Memory Node

└ Atmosphere Seed
```

---

# 9. Aurora Core Data Flow

完整链路：

```
Audio Engine

↓

Emotion Graph

↓

Atmosphere Generator

↓

Semantic Tokens

↓

Aurora Core

↓

Human Perception
```

---

# 10. Aurora Core Render Pipeline

```
Experience State

↓

Runtime State

↓

Lifecycle

↓

Token Resolver

↓

Material System

↓

Shader Layer

↓

Render Output
```

---

# 11. Aurora Core State Model

## Experience State

用户感知：

```
Idle

Playing

Paused

Gathering

Transition

Recall
```

---

## Runtime State

系统状态：

```
Initializing

Analyzing

Rendering

Restoring

Sleeping
```

---

# 12. Aurora Core Lifecycle

```
Birth

↓

Active

↓

Quiet

↓

Transform

↓

Recall

↓

Rest
```

---

说明：

## Quiet

低活动存在。

例如：

暂停音乐。

## Rest

完全休眠。

例如：

Aurora 未运行。

---

# 13. Interaction Philosophy

Aurora Core 遵循：

# Observe Before Interact

交互层级：

```
Observe

↓

Approach

↓

Interact

↓

Transform
```

---

核心输入：

必须支持：

* Mouse
* Keyboard

增强：

* Touchpad Gesture

---

# 14. Figma Architecture

最终结构：

```
Aurora Core


Variants:

├ Size

├ State

├ Context

└ Accessibility


Properties:

├ Atmosphere

├ Presence

├ Memory

├ Transition

└ Motion
```

---

避免：

Variant Explosion。

---

# 15. Semantic API

禁止暴露：

```qml
glowRadius

blurAmount

colorValue
```

---

采用：

语义 API：

```qml
AuroraCore {

    state: Playing

    context: MusicSpace

    atmosphere: Dream

    presenceLevel: 0.8

    memoryState: false

    transitionMode: None

    motionMode: Normal

}
```

---

# 16. Final Definition

## Aurora Core

> Aurora Core is the smallest perceptual unit that transforms invisible musical states into human experience.

中文：

> Aurora Core 是 Aurora 将不可见音乐状态转化为人类可感知体验的最小生命单元。

---

# 17. Component Library Progress

当前状态：

```
Aurora Design System v1.1


Foundation

✓


Aurora Core

✓ Production Primitive


Next:

Aurora Moment
```

---

# 18. Aurora Moment v1.0 Preview

下一组件正式定义：

# Memory Primitive

取消：

```
Card
```

采用：

```
Experience Object
```

---

Aurora Moment 定义：

> 一个由音乐、时间、情绪、环境和个人意义共同形成，并能够被再次进入的体验对象。

---

# Aurora Moment Object Model

```json
{
    MusicIdentity,

    TemporalIdentity,

    EmotionalIdentity,

    ContextIdentity,

    MeaningIdentity,

    MemoryIdentity
}
```

---

# Aurora Moment 核心问题

Aurora Core：

> 音乐现在是什么状态。

Aurora Moment：

> 为什么这一刻值得被记住。

---

# Aurora Moment v1.0 Design Scope

下一阶段：

## 01 Object Definition

定义 Moment 的本质。

---

## 02 Identity System

建立：

```
Music Identity

+

Temporal Identity

+

Emotion Identity

+

Context Identity

+

Meaning Identity

+

Memory Identity
```

---

## 03 Geometry System

解决：

为什么它不是普通 Card。

定义：

* Shape Language
* Optical Center
* Visual Weight
* Memory Depth
* Spatial Presence

---

## 04 Atmosphere Layer

数据流：

```
Emotion Graph

↓

Atmosphere Parameter

↓

Moment Visual
```

---

## 05 Memory State

```
Played

↓

Observed

↓

Remembered

↓

Meaningful
```

---

## 06 Recall Relationship

```
Moment

↓

Memory

↓

Recall

↓

Music Space
```

---

## 07 Transform System

核心：

```
Aurora Moment

↓

Aurora Core

↓

Music Space
```

---

## 08 Figma Architecture

定义：

* Component
* Variant
* Property
* Material
* State
* Interaction

---

## 09 QML Mapping

目标：

```qml
AuroraMoment {

    musicIdentity

    emotion

    atmosphere

    memoryState

    meaning

}
```

---

# Final Archive Statement

Aurora Core 完成：

> 音乐状态的存在表达。

Aurora Moment 下一阶段完成：

> 音乐记忆的身份表达。

Aurora 正式进入：

# Phase 02 — Component Library

下一核心组件：

# Aurora Moment v1.0

