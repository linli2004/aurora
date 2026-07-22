# Aurora Design System v1.1
## Component Library — Initial Development Baseline

**范围：** Aurora Foundation、Aurora Core v1.1、Aurora Moment v1.1、Aurora Crystal v1.0

**文档状态：** Initial Development Handoff / Baseline 0.1

**日期：** 2026-07-21

---

## 目录

- 0. 文档控制与开发结论
- 1. Aurora 组件宪法与冻结边界
- 2. Aurora Foundation v1.1 开发基线
- 3. 共享 Runtime、数据与信任架构
- 4. Aurora Core v1.1 初步实现契约
- 5. Aurora Moment v1.1 初步实现契约
- 6. Aurora Crystal v1.0 初步实现契约
- 7. 跨 Primitive 组合与 Transition 契约
- 8. 初步开发范围与实施顺序
- 9. 推荐仓库与模块结构
- 10. QML 与操作接口基线
- 11. 测试、视觉参考与质量门禁
- 12. 初步开发检查清单
- 13. 风险与工程护栏
- 14. 已冻结决策与待评审事项
- 15. 初步开发完成定义
- 附录 A — 建议共享枚举
- 附录 B — 开发状态总表

---

# 0. 文档控制与开发结论

> **文档定位**
> 本文件是 Foundation、Aurora Core、Aurora Moment 与 Aurora Crystal 的统一初步开发基线。它面向设计、Qt/QML、Runtime、数据模型与测试人员，用于开始第一轮实现；它不替代各组件的完整 Specification Archive。

| 对象 | 正式分类 | 当前状态 | 初步开发决策 |
| --- | --- | --- | --- |
| Aurora Foundation | Production Foundation | 已冻结 | 直接作为 Token 与共享规则基线 |
| Aurora Core v1.1 | Experience Runtime Primitive | Production Primitive | 作为第一优先级实现 |
| Aurora Moment v1.1 | Memory Primitive | Specification Archive | 按冻结数据与状态模型实现 MVP |
| Aurora Crystal v1.0 | Music Identity Primitive | Design Specification Complete | 允许原型实现；Identity Invariants 仍需视觉评审 |
| Aurora Presence | 待定义 | Deferred | 不进入本轮初步开发 |

**初步开发目标：**建立可运行的 QML 组件骨架、共享 Token Resolver、类型化状态、只读数据绑定、基础材质降级路径，以及可重复的视觉快照测试。

**本轮不追求：**完整 Emotion Engine、自动 Memory Association、最终 Shader 品质、跨设备同步、GNOME Shell 深度集成或生产数据库迁移。

---

# 1. Aurora 组件宪法与冻结边界

## 1.1 Component Review Gate

所有 Aurora Component 必须回答三个问题：它表达什么状态？它传递什么意义？用户为什么需要交互？无法回答的对象不得进入 Component Library。

```text
State
    ↓
Meaning
    ↓
Interaction
```

## 1.2 共同体验原则

| 原则 | 工程含义 |
| --- | --- |
| State Before Action | 先表达对象状态与意义，再暴露操作。 |
| Observe Before Interact | 用户先感知、靠近，再触发操作或转换。 |
| Stable Geometry / Dynamic Expression | 基础几何稳定；变化由材质、光学、状态和环境承担。 |
| Meaning Belongs to Human | 系统可以建议关联，但不可替用户确认人生意义。 |
| Recognition Before Decoration | 音乐身份必须先可识别，再允许材质与视觉表达。 |
| Local First Intelligence | 默认使用本地数据与本地模型；自动生成内容必须具有 Provenance。 |

## 1.3 Aurora Primitive Composition Rule 01

```text
Moment supplies memory.
Crystal preserves identity.
Core expresses state.
Atmosphere creates environment.
```

| Primitive | 冻结职责 | 明确不负责 |
| --- | --- | --- |
| Aurora Core | Runtime State 与感知状态表达 | 音乐视觉身份、个人记忆意义 |
| Aurora Moment | Memory Identity、时间环境与个人意义 | 实时播放状态、歌曲视觉连续性 |
| Aurora Crystal | Music Identity 与跨 Context 识别连续性 | 用户记忆意义、运行状态 |
| Aurora Atmosphere | 情绪、光温与空间环境 | 对象身份和持久化数据 |

## 1.4 Aurora Runtime Constitution

**Lifecycle** 描述对象经历了什么；**Experience State** 描述用户感受到什么；**Runtime State** 描述系统正在执行什么。三者必须在数据模型、Figma Variant 与 QML 类型中分离。

```text
Lifecycle State
≠
Experience State
≠
Runtime State
```

---

# 2. Aurora Foundation v1.1 开发基线

Foundation 是所有 Primitive 的唯一共享视觉与语义来源。初步实现不得在组件内部散落未经 Token Resolver 管理的颜色、模糊、圆角、光晕或动画时长。

| Token Domain | 核心职责 | 初步开发最低实现 |
| --- | --- | --- |
| Color | 身份色、环境色、对比与温度 | Semantic Color Map + 高对比映射 |
| Typography | 人类时间、标题、辅助信息 | 标题/正文/标签三级样式 |
| Material | Surface、Glass、Memory、Identity | Eco 与 Balanced 两级材质 |
| Motion | 交互、状态变化与 Transform | Normal / Reduced / Static |
| Spatial | 尺寸、间距、层级与 Context | Core/Moment/Crystal 尺寸常量 |
| Accessibility | 运动、对比、输入与认知密度 | Reduced Motion、High Contrast、Keyboard |
| Semantic | State、Atmosphere、Memory Presence | 语义枚举到渲染参数的映射 |
| Optical | CenterBalance、LightWeight、DepthBias、VisualGravity | 每个 Primitive 提供 Optical Profile |
| Ambient Behavior | 存在而非动作 | AttentionLevel 及 Idle Drift 开关 |

## 2.1 Ambient Attention 基线

| 体验 | AttentionLevel |
| --- | --- |
| Idle Drift | 0.1 |
| Home Atmosphere | 0.2 |
| Playing | 0.4 |
| Recall | 0.7 |
| Transition | 0.8 |

## 2.2 Token Resolver 规则

```text
Semantic State
+ Context
+ Accessibility
+ Quality
+ Optical Profile
        ↓
Token Resolver
        ↓
Material / Motion / Shader Parameters
```

- 组件 API 暴露 `presenceLevel`、`memoryPresence`、`transitionMode` 等语义属性。
- 禁止向页面调用方暴露 `glowRadius`、`blurAmount`、`refractionAmount` 等绘制参数。
- Eco 模式必须牺牲特效而不是牺牲身份、状态或可读性。

---

# 3. 共享 Runtime、数据与信任架构

## 3.1 统一 Runtime Pipeline

```text
Data Object
    ↓
Lifecycle Resolver
    ↓
Experience State Mapper
    ↓
Trust / Provenance Resolver
    ↓
Semantic Token Resolver
    ↓
Material Resolver
    ↓
Runtime Renderer
```

## 3.2 Data / Presentation 分离

Core、Moment 与 Crystal 的数据对象由 Runtime 或 Repository 提供。QML 组件只声明呈现 Context、Size、Accessibility 和 Quality，不直接修改核心状态。

```text
Domain Data (read-only)
≠
QML Presentation State
≠
User Intent / Operation
```

## 3.3 类型化 API

- 所有状态、Context、Size、Availability、Ownership 与 Preservation Level 使用枚举。
- 禁止使用 `"Primary"`、`"Remembered"` 等字符串驱动状态。
- 用户操作通过明确命令进入 Trust、Lifecycle 或 Identity Resolver。

## 3.4 Provenance 最低契约

```text
{
  source,
  processingLocation,
  generatedBy,
  userConfirmed,
  lastUpdated,
  modelVersion
}
```

任何自动形成的 Moment 或自动生成的 Crystal Identity Seed 都必须能够解释数据来源、处理位置、系统推断、用户确认以及删除或修改路径。

## 3.5 Availability 共同语义

| 状态 | 含义 | 关键行为 |
| --- | --- | --- |
| Active | 对象和来源均可用 | 完整呈现与交互 |
| Archived | 退出日常主动展示 | 对象与 Recall 保留 |
| Detached | 来源当前不可用 | 身份/记忆保留，可 Relink |
| Deleted | 用户明确删除对象 | 从正常体验移除 |
| Unavailable | 必要身份信息无法解析 | 使用可信 Fallback，不伪造内容 |

---

# 4. Aurora Core v1.1 初步实现契约

> **正式分类**
> Aurora Core 是 **Experience Runtime Primitive**：将不可见的音乐运行状态转换为人类可感知的最小生命单元。

## 4.1 职责与系统位置

```text
Audio / Runtime State
        ↓
Emotion & Atmosphere Semantic Input
        ↓
Aurora Core
        ↓
Human Perception
```

## 4.2 Experience State

| 状态 | 用户含义 | 初步视觉要求 |
| --- | --- | --- |
| Idle | Quiet Presence | 低光、静态深度、极低 Ambient Drift |
| Playing | 音乐正在存在 | Emotion-driven Motion，不使用简单 Beat Pulse |
| Paused | 关系保持但运动停止 | Frozen Light，Presence 保留 |
| Gathering | 系统正在聚集音乐状态 | 聚光与柔和能量汇聚 |
| Transition | 对象或空间正在变化 | Expand / Collapse / Merge / Restore |
| Recall | 记忆状态正在恢复 | Memory Trace 与旧光线出现 |

## 4.3 Runtime State

```text
Initializing
Analyzing
Rendering
Transitioning
Sleeping
Failure
Recovery
```

## 4.4 Lifecycle

```text
Birth → Active → Quiet → Transform → Recall → Rest
```

## 4.5 Geometry 与 Material

- 基础几何：**Soft Organic Circle**。
- 原则：Geometry Stable，Expression Dynamic。
- Material 层：Core Surface + Inner Semantic Light + Edge Bloom。
- 初步实现至少提供 64、96、128 px 三个尺寸；Immersive 尺寸可后续增加。

## 4.6 QML Baseline

```text
AuroraCore {
    state: AuroraCore.Playing
    context: AuroraContext.MusicSpace
    atmosphere: AuroraAtmosphere.Dream
    presenceLevel: 0.8
    memoryState: false
    transitionMode: AuroraTransition.None
    motionMode: AuroraMotion.Normal
}
```

## 4.7 MVP 验收

- 六个 Experience State 均可由类型化状态切换。
- Reduced Motion 与 Static 模式无持续高频动画。
- Keyboard 可触发 Play/Pause 意图；组件本身不直接控制 Audio Engine。
- Eco 渲染路径在集成显卡上保持稳定；状态识别不依赖 Shader。
- 完成 `Idle`、`Playing Calm`、`Playing Energy`、`Recall`、`Transition Expand` 快照。

---

# 5. Aurora Moment v1.1 初步实现契约

> **正式分类**
> Aurora Moment 是 **Memory Primitive**：保存音乐体验身份、环境与个人意义，并允许用户重新召回和进入。

## 5.1 冻结对象模型

```text
AuroraMomentData
├ MusicIdentityReference
├ TemporalIdentity
├ EmotionalIdentity
├ ContextIdentity
├ SuggestedMeaning
├ ConfirmedMeaning
├ MemoryIdentity
├ MemoryWeight
└ Provenance
```

## 5.2 Temporal Identity

```text
{
  timestamp,
  localTimezone,
  periodSemantic: { season, dayPart },
  periodLabelOverride
}
```

`periodSemantic` 根据 Locale 重新生成人类时间；只有用户主动编辑时才持久保存 `periodLabelOverride`。

## 5.3 Meaning Identity

```text
{
  suggestedMeaning: { relation, source, confidence },
  confirmedMeaning: { userNote, personalTags, importance, confirmedAt }
}
```

未经用户确认的 Suggested Meaning 不得进入正式 Meaning 展示层，也不得被写成用户事实。

## 5.4 Memory Identity 与 Weight

| 对象 | 回答的问题 | 使用位置 |
| --- | --- | --- |
| Memory Confidence | 系统多确定该体验有资格形成 Moment？ | Lifecycle Eligibility |
| Memory Weight.Intensity | 当时情绪变化多强？ | Semantic Memory Presence |
| Memory Weight.Depth | 关系随时间形成多深？ | Temporal Layer 与 Spatial Weight |
| Memory Weight.PersonalMeaning | 用户主动赋予多高重要性？ | 稳定性与深度，不直接增加亮度 |

## 5.5 三层状态

| Memory Lifecycle | Experience State | 典型 Runtime State |
| --- | --- | --- |
| Created | Dormant | Binding |
| Candidate | Dormant / Present | Ready |
| Observed | Present | Rendering |
| Remembered | Remembered | Ready |
| Meaningful | Meaningful | Ready |
| Recalled | Recalling | Restoring |
| Archived | Archived | Sleeping / Ready |

## 5.6 Geometry、层级与 Material

- 基础几何：**Soft Organic Square**，1:1。
- 尺寸：Compact 240、Primary 400、Recall 640。
- 圆角约为短边的 16%。
- 层级：Surface Identity → Atmosphere Field → Music Identity Reference → Temporal Trace → Memory Material → Meaning Indicator。
- Memory Material 表达时间重量，不得呈现为宝石、奖杯或稀有度卡片。

## 5.7 用户操作

```text
keepMoment()
updateMeaning()
confirmSuggestedMeaning()
archiveMoment()
restoreMoment()
deleteMoment()
relinkMusicSource()
```

## 5.8 QML Baseline

```text
AuroraMoment {
    moment: momentRepository.currentMoment
    size: AuroraMoment.Primary
    context: AuroraContext.Home
    accessibilityMode: AuroraAccessibility.Normal
    recallEnabled: true
}
```

## 5.9 MVP 验收

- 使用 Mock Repository 提供只读 Moment Data。
- 完成 Present、Remembered、Meaningful、Recalling、Archived、Detached 六个可见状态。
- 实现 Suggested Meaning 与 Confirmed Meaning 的清晰分离。
- Detached 时保留时间、意义、Memory Trace 与 Crystal 引用，并提供 Relink 意图。
- 完成 Home、Memory Flow、Recall、Archive 四个 Context 快照。
- 完成基本导出/导入 Fixture，验证 Timestamp、Meaning、Ownership 与 Provenance 不丢失。

---

# 6. Aurora Crystal v1.0 初步实现契约

> **当前成熟度**
> Aurora Crystal 已完成 Design Specification，可开始受限原型开发；其 Identity Invariants 与真实 Artwork 视觉关系仍需下一轮评审后冻结。

## 6.1 正式分类与最高规则

Aurora Crystal 是 **Music Identity Primitive**。最高规则是 **Recognition Before Decoration**：隐藏所有文字后，用户仍应能够识别同一个音乐对象。

## 6.2 Identity Invariants

| Invariant | 职责 | 跨 Context 要求 |
| --- | --- | --- |
| Artwork Anchor | 关键构图与识别焦点 | 裁切、折射和尺寸变化后仍可识别 |
| Color Signature | 稳定的身份色组合 | 不得被 Moment Atmosphere 完全覆盖 |
| Shape Signature | 内部构图轮廓 | 影响遮罩和高光，不改变外部基础几何 |
| Light Signature | 固有光线方向 | 不承担 Playing / Paused 状态 |
| Texture Signature | Artwork 与音乐空间质感 | 避免默认 Chrome、Neon、Gem Facet |
| Optical Gravity | 视觉识别中心 | Transform 中不得跳跃 |
| Spatial Character | Wide / Close / Floating / Dense 等 | 影响深度与空间展开 |

## 6.3 Object Model

```text
AuroraCrystalData
├ MusicIdentity
├ IdentitySignature
│  ├ ArtworkAnchor
│  ├ ColorSignature
│  ├ ShapeSignature
│  ├ LightSignature
│  ├ TextureSignature
│  └ OpticalGravity
├ ArtworkState
├ AvailabilityState
└ Provenance
```

## 6.4 Geometry 与 Material

- 基础几何：**Soft Identity Square**，1:1，圆角约短边 10%。
- Moment 内部建议尺寸 160；Music Space 初步建议 420。
- Material：Artwork Surface + Soft Refraction + Identity Light + Edge Bloom + Depth Veil。
- Soft Refraction 仅用于轻微深度和 Transform，不使用棱镜彩虹或大范围扭曲。
- Recognition Safe Region 至少保留内部 76% × 76%，Core 和状态图标不得遮挡。

## 6.5 Experience / Runtime

| Experience State | 用户含义 | 典型 Runtime |
| --- | --- | --- |
| Dormant | 身份存在但不活跃 | Ready / Sleeping |
| Present | 默认可识别 | Ready |
| Focused | 当前选择或接近 | Rendering |
| Immersive | Music Space 主要身份 | Ready |
| Transitioning | 跨 Context 身份连续变化 | Rendering |
| Detached | 音频源不可用但身份保留 | Relinking / Ready |
| Unavailable | 身份信息不足 | Failure / GeneratingIdentity |

## 6.6 Identity Seed

无 Artwork 时生成 Aurora Identity Seed。它是可区分的系统身份标记，不是假装成为官方专辑封面。来源包括 Track、Artist、Album、Audio Profile 与 Color Signature。

## 6.7 QML Baseline

```text
AuroraCrystal {
    crystal: crystalRepository.currentCrystal
    size: AuroraCrystal.MusicSpace
    context: AuroraContext.MusicSpace
    experienceState: AuroraCrystal.Immersive
    accessibilityMode: AuroraAccessibility.Normal
    qualityMode: AuroraQuality.Adaptive
}
```

## 6.8 MVP 验收

- 同一 Fixture 在 Home、Moment、Recall 与 Music Space 中保持 Artwork Anchor、Color Signature、Optical Gravity。
- 无 Artwork Fixture 能生成明确标记为 Generated Identity 的 Fallback。
- Detached 后保留视觉身份并提供 `relinkSource()`。
- Eco 模式关闭 Refraction 后仍可识别。
- 完成 Present.Home、Present.Moment、Immersive.MusicSpace、Transitioning、Detached.Recall 快照。

---

# 7. 跨 Primitive 组合与 Transition 契约

## 7.1 页面组合

| 体验场景 | Moment | Crystal | Core | Atmosphere |
| --- | --- | --- | --- | --- |
| Home Current Experience | 可选：最近 Moment 语境 | 当前音乐身份 | Idle / Playing / Paused | 当前环境 |
| Moment Object | 主对象 | 内部音乐身份引用 | 通常隐藏或最小化 | 保存的环境 |
| Recall | 恢复记忆与时间 | 恢复音乐身份 | Recall / Gathering | 恢复旧环境 |
| Music Space | 提供进入来源与记忆上下文 | 主视觉身份 | 表达实时状态 | 完整展开 |
| Detached Memory | 记忆完整保留 | Detached Identity | 不可播放或静默 | 低 Presence |

## 7.2 Moment → Music Space 标准链路

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

| 阶段 | 不可丢失对象 |
| --- | --- |
| Boundary Dissolves | Moment Atmosphere、Temporal Trace |
| Crystal Emerges | Artwork Anchor、Music Identity、Optical Gravity |
| Core Awakens | Gathering、Playing 或 Recall 状态 |
| Atmosphere Expansion | Emotion Color、Light Temperature、Spatial Behavior |

## 7.3 初步动画策略

- 第一轮使用属性动画与简化 Shader，实现身份连续性验证，而非追求最终材质。
- Reduced Motion 使用 Cross-dissolve + Stable Anchor，不进行深度折射。
- 禁止 `Moment Fade Out → Player Fade In` 作为最终交互结构。
- Transform 失败时回退为可访问的即时 Context 切换，同时保持 Crystal 身份。

---

# 8. 初步开发范围与实施顺序

## 8.1 推荐顺序

| 阶段 | 交付内容 | 进入条件 |
| --- | --- | --- |
| Stage A — Shared Foundation | Token、Enums、Context、Accessibility、Quality、Resolver Stub、Fixture | 仓库可构建，单元测试运行 |
| Stage B — Aurora Core MVP | 六个 Experience State、三种 Motion Mode、Eco/Balanced | 核心快照与键盘测试通过 |
| Stage C — Aurora Moment MVP | 只读 Data、状态映射、Meaning、Archived/Detached、Recall 意图 | 状态映射与序列化 Fixture 通过 |
| Stage D — Aurora Crystal MVP | Artwork Anchor、Identity Signature、Fallback、Detached | 跨 Context 识别快照通过 |
| Stage E — Integrated Flow | Moment → Crystal → Core → Atmosphere Transform | 完整原型场景可运行 |
| Stage F — Refinement Gate | 可访问性、性能、视觉参考、迁移测试 | 决定是否进入 Production Refinement |

## 8.2 初步开发 In Scope

- Qt 6 / QML 可运行组件与模块结构。
- Mock Audio Runtime、Mock Moment Repository、Mock Crystal Repository。
- 类型化 Experience / Runtime / Lifecycle 映射。
- Foundation Token Resolver 的本地静态实现。
- Eco 与 Balanced 两级渲染；Immersive 仅保留接口。
- Keyboard、Reduced Motion、High Contrast 基础支持。
- 自动视觉快照命名与基线目录。
- 基础 JSON Fixture 与序列化/迁移测试。

## 8.3 初步开发 Deferred

- 真实 Audio Analysis 与 Emotion Graph。
- 自动 Memory Association 和生产 Confidence 算法。
- 跨设备同步及云端 Provider。
- 最终 GPU Shader、复杂 Refraction 与粒子系统。
- Aurora Presence 与 GNOME Top Panel 集成。
- 生产数据库、加密导出和完整 Privacy Center。
- Touchpad 高级 Gesture。

## 8.4 端到端原型场景

```text
1. 加载本地 Mock Track
2. Crystal 解析并显示音乐身份
3. Core: Idle → Gathering → Playing → Paused
4. 用户执行 Keep This Moment
5. Moment 进入 Remembered，并添加 Confirmed Meaning
6. 用户触发 Recall Moment
7. Moment Boundary Dissolves
8. Crystal Emerges
9. Core Awakens
10. Atmosphere Expansion → Music Space
11. 模拟音频文件移动，Moment / Crystal 进入 Detached
12. Relink 后恢复
```

---

# 9. 推荐仓库与模块结构

```text
AuroraDesignSystem/
├ CMakeLists.txt
├ docs/
│  ├ specifications/
│  └ development-baseline/
├ src/
│  ├ foundation/
│  │  ├ tokens/
│  │  ├ semantics/
│  │  ├ optical/
│  │  └ accessibility/
│  ├ runtime/
│  │  ├ TokenResolver/
│  │  ├ LifecycleResolver/
│  │  ├ StateMapper/
│  │  ├ TrustResolver/
│  │  └ IdentityInvariantGuard/
│  ├ data/
│  │  ├ CoreData/
│  │  ├ MomentData/
│  │  └ CrystalData/
│  ├ components/
│  │  ├ Core/
│  │  ├ Moment/
│  │  └ Crystal/
│  └ demo/
│     └ InitialDevelopmentFlow/
├ tests/
│  ├ unit/
│  ├ state-mapping/
│  ├ serialization/
│  ├ accessibility/
│  └ snapshots/
└ fixtures/
   ├ tracks/
   ├ moments/
   └ identities/
```

## 9.1 建议 QML Module

| Module | 用途 |
| --- | --- |
| Aurora.Foundation 1.1 | Token、Semantic、Optical、Accessibility |
| Aurora.Runtime 1.0 | Resolver、Mapper、Guard 与 Operation Interface |
| Aurora.Data 1.0 | 只读 Core/Moment/Crystal Data Types |
| Aurora.Components 1.0 | AuroraCore、AuroraMoment、AuroraCrystal |
| Aurora.Demo 0.1 | 初步开发原型场景 |

## 9.2 命名规则

- 类名使用 `AuroraCore`、`AuroraMoment`、`AuroraCrystal`。
- 只读数据类型以 `Data` 结尾；Repository 提供对象引用。
- Resolver 不直接依赖页面；页面只依赖语义输出。
- 快照采用 `Component.State.Context.Accessibility.Quality` 格式。

---

# 10. QML 与操作接口基线

## 10.1 组合示例

```text
Item {
    AuroraMoment {
        id: momentView
        moment: momentRepository.selectedMoment
        size: AuroraMoment.Primary
        context: AuroraContext.Home
        recallEnabled: true
        onRecallRequested: memoryController.recall(moment.id)
    }

    AuroraCrystal {
        id: crystalView
        crystal: crystalRepository.forMusic(momentView.moment.musicIdentity)
        size: AuroraCrystal.Moment
        context: AuroraContext.Moment
    }

    AuroraCore {
        id: coreView
        state: audioRuntime.experienceState
        context: AuroraContext.Home
        presenceLevel: audioRuntime.presenceLevel
    }
}
```

## 10.2 操作边界

| 操作 | 所属服务 | UI 结果 |
| --- | --- | --- |
| keepMoment() | Memory Service | 返回更新后的只读 Moment Data |
| updateMeaning() | Memory + Trust | 修改 Confirmed Meaning 与 Provenance |
| archiveMoment() | Memory Lifecycle Resolver | Experience State 映射为 Archived |
| deleteMoment() | Trust + Persistence | 执行明确删除流程 |
| relinkMusicSource() | Identity / Library Service | Detached → Active |
| setArtworkOverride() | Identity Service | 更新 Crystal ArtworkState |
| requestPlayPause() | Audio Runtime | Core 状态由 Runtime 回推 |

## 10.3 只读要求

UI 不得通过属性赋值改变 Memory Engine、Identity Resolver 或 Audio Runtime 的核心字段。所有变更必须先表达为用户意图，再由服务返回新的只读对象或状态。

---

# 11. 测试、视觉参考与质量门禁

## 11.1 优先级

| 级别 | 必须完成 |
| --- | --- |
| P0 — 原型可运行 | 构建、类型化状态、Fixtures、Core/Moment/Crystal 基础渲染 |
| P1 — 初步开发完成 | 状态映射、Detached、Meaning 分离、跨 Context 身份、Reduced Motion、快照 |
| P2 — Production Refinement | 真实性能、完整 A11y、Localization、Privacy、Migration、最终 Shader |

## 11.2 快照最低集合

```text
Core
├ AuroraCore.Idle.Home.Normal.Balanced
├ AuroraCore.PlayingCalm.MusicSpace.Normal.Balanced
├ AuroraCore.PlayingEnergy.MusicSpace.Normal.Balanced
├ AuroraCore.Recall.Recall.ReducedMotion.Eco
└ AuroraCore.TransitionExpand.MusicSpace.Normal.Balanced

Moment
├ AuroraMoment.Present.Home.Normal.Balanced
├ AuroraMoment.Remembered.MemoryFlow.Normal.Balanced
├ AuroraMoment.Meaningful.Recall.Normal.Balanced
├ AuroraMoment.Recalling.Recall.ReducedMotion.Eco
├ AuroraMoment.Archived.Archive.Normal.Eco
└ AuroraMoment.Detached.MemoryArchive.HighContrast.Eco

Crystal
├ AuroraCrystal.Present.Home.Normal.Balanced
├ AuroraCrystal.Present.Moment.Normal.Balanced
├ AuroraCrystal.Immersive.MusicSpace.Normal.Balanced
├ AuroraCrystal.Transitioning.MusicSpace.ReducedMotion.Eco
├ AuroraCrystal.Detached.Recall.HighContrast.Eco
└ AuroraCrystal.Unavailable.Search.Normal.Eco
```

## 11.3 状态映射测试

- 每个 Lifecycle 状态只能映射到允许的 Experience State。
- Runtime Failure 不得伪装为 Archived、Detached 或 Recalling。
- Memory Confidence 只影响 Lifecycle Eligibility，不影响视觉亮度。
- Personal Meaning 通过 Depth、Trace 与 Stability 表达，不直接映射最大 Glow。
- Crystal Context Transform 必须通过 Identity Invariant Guard。

## 11.4 初步性能预算

| 项目 | 初步目标 |
| --- | --- |
| Core / Moment / Crystal 静态 | 60 FPS，Eco 与 Balanced 均稳定 |
| 单个 Transform | 无明显主线程阻塞 |
| Memory Flow | 至少 30 个 Moment 使用虚拟化列表 |
| Artwork Cache | 避免 Context 切换重复解码 |
| Reduced Motion | 关闭持续 Refraction、Drift 与复杂深度动画 |

---

# 12. 初步开发检查清单

## 12.1 Foundation 与 Runtime

- 建立 QML Modules 与 CMake 构建。
- 定义全部共享枚举与 Context。
- 实现 Token Resolver Stub 与 Eco/Balanced Profile。
- 实现 Accessibility Resolver。
- 实现 Snapshot Harness 与稳定 Fixture。

## 12.2 Core

- 完成基础圆形、Inner Light、Edge Bloom。
- 完成六个 Experience State。
- 完成 Experience / Runtime 分离。
- 完成 Play/Pause Intent 信号与键盘支持。

## 12.3 Moment

- 建立只读 AuroraMomentData 与 Mock Repository。
- 实现 Temporal Semantic 本地化生成器。
- 实现 Suggested / Confirmed Meaning 分离。
- 实现 Lifecycle → Experience Mapper。
- 实现 Archived、Detached 与操作接口。
- 实现 JSON Fixture、导出/导入与版本字段。

## 12.4 Crystal

- 建立 Artwork Anchor 与 Recognition Safe Region。
- 实现 Color Signature 与 Optical Gravity Fixture。
- 实现 Identity Seed Fallback。
- 实现 Detached / Relink。
- 实现 Home、Moment、Recall、Music Space 四个 Context。

## 12.5 集成

- 实现 Moment → Crystal → Core → Atmosphere 的分阶段 Transform。
- 添加 Reduced Motion 回退。
- 验证身份、状态、记忆与环境连续性。
- 记录所有初步实现偏差，回写 Refinement Review。

---

# 13. 风险与工程护栏

| 风险 | 典型错误 | 护栏 |
| --- | --- | --- |
| 视觉过度设计 | 所有对象持续漂移、发光、折射 | Ambient Attention 受控；Eco 为默认可运行路径 |
| 状态泄漏 | 数据库 Candidate 直接成为 Figma Variant | Lifecycle → Experience Mapper |
| 身份漂移 | 不同页面完全重绘 Artwork | Identity Invariant Guard + 快照 |
| 意义越权 | 系统推断直接写入用户意义 | Suggested / Confirmed Meaning 分离 |
| 直接数据修改 | QML 赋值改变 Memory 状态 | 只读 Data + Operation Interface |
| 本地文件丢失即记忆消失 | 删除音频导致 Moment 删除 | Detached 状态与 Relink |
| 字符串状态 | 拼写错误导致隐式回退 | 类型化枚举 |
| 性能降级破坏身份 | 关闭 Shader 后无法识别 | Identity 优先于 Material |
| 国际化固化文本 | 永久保存英文 Period Label | Semantic Time + Locale 生成 |
| 算法升级重写用户事实 | 新模型覆盖 Confirmed Meaning | Provenance、版本与用户确认保护 |

---

# 14. 已冻结决策与待评审事项

## 14.1 已冻结

- Core = Runtime State；Moment = Memory Identity；Crystal = Music Identity。
- Primitive Composition Rule 01。
- Lifecycle / Experience / Runtime 三层分离。
- Meaning Belongs to Human。
- Moment 的 Provenance、Temporal Semantic、Suggested/Confirmed Meaning 与 Detached。
- Data / Presentation 分离与类型化 QML API。
- Moment → Crystal → Core → Atmosphere 的 Transform 顺序。
- Foundation 九类 Token。

## 14.2 初步开发中允许调整

- 组件内部 QML 文件拆分与私有实现。
- Token 数值、动画具体时长与 Shader 参数。
- Mock Repository 和 Fixture 格式。
- Snapshot 工具链与 CI 集成方式。
- Crystal Identity Signature 的生成算法原型。

## 14.3 进入 Production 前必须再次评审

- Aurora Crystal Identity Invariants 的真实视觉样本。
- Moment 与 Crystal 在复杂 Artwork 上的遮挡与识别安全区。
- Core 在 Integrated GPU 与 Low Power 下的 Shader 策略。
- Memory Persistence、Migration、Delete 与 Export 行为。
- 完整 Accessibility 与 Privacy Center 连接。

---

# 15. 初步开发完成定义

> **Definition of Done — Initial Development**
> 初步开发完成并不等于 Production。它表示共享架构已在真实 QML 中成立，三个 Primitive 可以用稳定 Fixture 运行，主要状态与身份连续性可通过自动测试和视觉快照验证。

- 仓库可以从空环境完成配置、构建并启动 Demo。
- Core、Moment、Crystal 使用相同 Foundation、Context、Accessibility 和 Quality 类型。
- 所有核心状态均由枚举和 Resolver 驱动。
- Moment 与 Crystal 数据对象只读，修改通过明确操作。
- 端到端原型场景完整运行，包括 Detached 与 Relink。
- P0/P1 快照、状态映射和基础可访问性测试通过。
- 已知视觉与架构偏差形成明确 Refinement Backlog。

```text
Initial Development Complete
≠
Production Primitive

Initial Development Complete
=
Architecture Proven in Running Code
```

---

# 附录 A — 建议共享枚举

```text
AuroraContext
  Home, Moment, MemoryFlow, Recall, MusicSpace, Archive, Search, Queue

AuroraAccessibility
  Normal, ReducedMotion, HighContrast, CognitiveMinimal

AuroraQuality
  Eco, Balanced, Immersive, Adaptive

AuroraMotion
  Normal, Reduced, Static

AuroraTransition
  None, Expand, Collapse, Merge, Restore

MomentAvailability
  Active, Archived, Detached, Deleted

MemoryOwnership
  SystemSuggested, UserAccepted, PersonalMemory

PreservationLevel
  Temporary, Remembered, Important, Permanent
```

---

# 附录 B — 开发状态总表

| 对象 | 规范状态 | 开发状态 | 下一门禁 |
| --- | --- | --- | --- |
| Foundation | Production Foundation | 可直接实现 | Token 与 Resolver 快照 |
| Aurora Core v1.1 | Production Primitive | MVP 优先 | QML / Performance Regression |
| Aurora Moment v1.1 | Specification Archive | MVP 可开始 | QML、Snapshot、A11y、Persistence |
| Aurora Crystal v1.0 | Design Specification | 受限原型 | Identity Review、Visual Reference、Production Refinement |
| Aurora Presence | 未开始 | Deferred | Crystal 稳定后启动 |

---
