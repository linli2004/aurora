# Aurora Music Framework
## 项目计划与 UI 方案

**版本：v1.0（Demo Sprint + 长期路线）**  
**状态基线：AUR-029 已完成本地验收**  
**目标平台：Ubuntu / GNOME / Qt 6 / QML / C++**

---

# 1. 项目定义

## 1.1 产品名称

**Aurora Music Framework**

## 1.2 产品定位

Aurora 不是传统音乐播放器。

Aurora 是：

> 一个理解用户、理解音乐，并创造个人音乐氛围的 Ubuntu 原生音乐环境。

系统核心：

```text
Music
+
Memory
+
Intelligence
+
Atmosphere
+
Desktop
```

Aurora 的价值不在于“提供更多播放器按钮”，而在于把一次音乐播放转化为：

```text
音乐身份
+
个人时刻
+
环境氛围
+
长期记忆
+
可再次进入的体验
```

---

# 2. 核心设计原则

## 2.1 Music First

音乐永远是主角。

```text
Music
→ Emotion
→ Atmosphere
→ Experience
```

视觉不能抢夺音乐，也不能成为技术展示。

## 2.2 Human First

Aurora 服务的是“人与音乐的关系”，不是展示算法能力。

## 2.3 Invisible Intelligence

用户应该感受到：

> Aurora understands.

而不是看到：

> AI is working.

AI 不直接操作 UI，只输出语义参数。

## 2.4 Continuous Experience

Aurora 不使用传统页面跳转作为核心体验。

```text
Home
→ Moment
→ Crystal
→ Core
→ Atmosphere
→ Music Space
→ Memory
```

对象应连续变形，而不是一个页面消失、另一个页面出现。

## 2.5 Memory Belongs to User

```text
Your music.
Your moments.
Your memory.
Your device.
```

默认 Local First，本地处理，本地存储，未来同步只能是可选项。

## 2.6 Respect the Desktop

Aurora 存在于 Ubuntu 中，但不替代 Ubuntu。

```text
Influence, Not Control
```

---

# 3. 核心对象职责

## 3.1 Aurora Core

```text
Aurora Core = Runtime State
```

回答：

> 音乐现在是什么状态？

负责：

- Playing
- Paused
- Gathering
- Recalling
- Restoring
- Sleeping
- 用户的 Play / Pause 意图

不负责：

- 歌曲身份
- 记忆
- 音频分析
- 页面导航

## 3.2 Aurora Moment

```text
Aurora Moment = Memory Identity
```

回答：

> 为什么这一刻值得被留下？

负责：

- 音乐体验身份
- 时间语义
- 用户 Meaning
- Memory Weight
- Provenance
- Recall 入口
- Detached 状态

不负责：

- 实时播放状态
- 完整音乐视觉身份
- 音频特征分析
- Music Space 渲染

## 3.3 Aurora Crystal

```text
Aurora Crystal = Music Identity
```

回答：

> 这是哪一个音乐存在？

负责：

- Artwork Anchor
- Color Signature
- Optical Gravity
- Light Signature
- Texture Signature
- 跨 Home / Moment / Recall / Music Space 的身份连续性

最高规则：

```text
Recognition Before Decoration
```

隐藏文字后，用户仍应认出同一音乐对象。

## 3.4 Atmosphere

```text
Atmosphere = Environment
```

负责：

- Color
- Light
- Motion
- Density
- Temperature
- Depth
- Spatial Behavior

Atmosphere 不是普通背景，也不是频谱可视化。

## 3.5 组合规则

```text
Moment supplies memory.
Crystal preserves identity.
Core expresses state.
Atmosphere creates environment.
```

---

# 4. 当前项目状态

## 4.1 已完成

### 本地音乐运行基础

- 本地多文件播放
- 队列、上一首、下一首
- 播放位置恢复
- 音量恢复
- MPRIS
- Metadata / Artwork fallback
- Audio Feature Analyzer
- SQLite 曲库
- 稳定 Track Identity
- Track / TrackSource 分离
- Missing Source
- Stable Relink
- 曲库目录持久化
- Rescan all
- 文件系统监听
- 自动刷新
- 去重播放列表
- Ubuntu desktop 安装与图标

### Moment 与 Memory

- Keep Moment
- Moment SQLite 持久化
- Home 显示真实 Moment
- 重启恢复
- Moment → Crystal → Music Space 召回
- 多 Moment Memory Flow
- Moment 选择
- Confirmed Meaning 编辑与清空
- Detached Moment
- Recall 后返回 Memory Flow

## 4.2 当前问题

当前产品功能已超过普通播放器，但第一眼仍像播放器，原因是：

- Music Space 默认长期显示控制 UI
- Crystal 仍像专辑封面卡片
- Atmosphere 只是背景层
- 控件层级高于空间体验
- Home 仍有原型和设计展示感
- 动效还没有形成完整的 Aurora Signature

## 4.3 当前判断

```text
架构成立
基础功能成立
Memory 第一闭环成立
产品视觉身份尚未成立
```

---

# 5. 两天 Demo 目标

## 5.1 Demo 的唯一叙事

```text
打开 Aurora
→ Home 出现一个属于用户的 Moment
→ 点击 Moment
→ 音乐身份连续展开
→ 进入沉浸式 Music Space
→ 画面随音乐呼吸
→ 用户接近时控制浮现
→ Keep Moment
→ 返回 Home
→ 新的记忆已经留下
```

Demo 不能试图展示所有功能。

Demo 必须让观众在 10 秒内理解：

> Aurora 不是播放器，而是音乐存在和记忆的空间。

## 5.2 两天内必须完成

### Day 1 — Music Space 视觉转向

1. Presence Mode
2. Control Mode
3. 自动隐藏控制
4. Hero Crystal
5. 音频响应 Atmosphere
6. 控件与空间层级重构

### Day 2 — 完整 Demo Story

1. Home 情绪入口
2. Moment → Music Space 主过渡精修
3. Keep Moment 反馈
4. 返回 Home 后新 Moment 出现
5. Release 稳定性
6. 录制 30–60 秒演示视频

## 5.3 Demo 暂停项

两天内不做：

- 云同步
- 在线音乐
- 歌词平台
- 社交
- 插件市场
- Memory 导入导出
- Archive / Delete 完整生命周期
- 完整 Intelligence
- Weather / Location
- Desktop Shell 扩展
- 复杂 Shader
- 完整设置中心

---

# 6. UI 总体结构

```text
Aurora App
│
├── Home
│   ├── Current / Latest Moment
│   ├── Natural Time
│   ├── Confirmed Meaning
│   └── Memory Entry
│
├── Memory Flow
│   ├── Moment Timeline
│   ├── Selected Moment
│   ├── Meaning Editor
│   └── Recall
│
├── Music Space
│   ├── Presence Mode
│   ├── Control Mode
│   ├── Hero Crystal
│   ├── Core
│   ├── Atmosphere
│   └── Queue / Library
│
└── Transition Layer
    ├── Moment Boundary Dissolves
    ├── Crystal Emerges
    ├── Core Awakens
    └── Atmosphere Expansion
```

---

# 7. Home UI 方案

## 7.1 Home 定义

Home 是：

> Aurora 每天打开后的第一句话。

Home 不是 Dashboard。

核心原则：

```text
One Moment First
```

## 7.2 Home 信息层级

```text
AURORA

This morning

[ Main Moment ]

失眠
Suki刘舒好

“那天终于安静下来了。”

Memory · 3
```

无用户 Meaning 时：

- 不显示系统生成的人生意义
- 不显示数据库技术文案
- 只显示时间、音乐身份和可进入性

## 7.3 Home 布局

```text
┌──────────────────────────────────────────────┐
│ AURORA                              Memory · 3│
│                                              │
│ This morning                                 │
│                                              │
│                 ┌──────────┐                 │
│                 │ Crystal  │                 │
│                 └──────────┘                 │
│                                              │
│                    失眠                      │
│                Suki刘舒好                    │
│                                              │
│        “那天终于安静下来了。”                │
│                                              │
│                  Enter                       │
└──────────────────────────────────────────────┘
```

## 7.4 Home 视觉要求

- 一次只突出一个 Moment
- Moment 不表现为普通卡片
- 背景与 Moment Atmosphere 有连续关系
- Memory 入口降级为次要动作
- Component Gallery 不出现在 Demo 默认界面
- 避免技术文案：
  - `A moment kept on this device`
  - `Local audio`
  - `Generated identity`

---

# 8. Memory Flow UI 方案

## 8.1 Memory Flow 定义

Memory Flow 不是 Listening History。

它是：

> 用户与音乐共同生活留下的痕迹。

## 8.2 Demo 版结构

```text
┌────────────────────────────────────────────────────┐
│ Back                    Memory Flow                 │
│                                                    │
│  Moments                    Selected Moment        │
│  ┌─────────────┐           ┌──────────────────┐    │
│  │ Morning     │           │ Hero Moment      │    │
│  │ 失眠        │           │                  │    │
│  └─────────────┘           │ 失眠             │    │
│  ┌─────────────┐           │ Suki刘舒好       │    │
│  │ Late Night  │           │                  │    │
│  │ Song B      │           │ Your meaning     │    │
│  └─────────────┘           │ [ textarea ]     │    │
│                            │ Save meaning     │    │
│                            │ Recall moment    │    │
│                            └──────────────────┘    │
└────────────────────────────────────────────────────┘
```

## 8.3 后续正式版

Memory Flow 将从时间列表升级为 Emotional Timeline：

```text
Summer Night
Blue Atmosphere
5 Moments
```

组织原则：

```text
时间负责定位
情绪与关系负责组织
```

---

# 9. Music Space UI 方案

## 9.1 Music Space 定义

Music Space 是：

> 音乐存在的空间。

不是：

> 播放器页面。

正式结构：

```text
Atmosphere Layer
↓
Aurora Crystal
↓
Song Identity
↓
Aurora Core
↓
Temporary Controls
```

---

# 10. Presence Mode

## 10.1 默认状态

进入 Music Space 后默认进入 Presence Mode。

显示：

- Atmosphere
- 大型 Hero Crystal
- 歌曲名称
- 极弱歌手信息
- Core
- 极弱进度感知

隐藏：

- Library
- Tracks
- 搜索
- 音量
- Previous / Next
- Diagnostics
- Change Music
- Keep Moment 主按钮
- 复杂播放列表

## 10.2 Presence Mode 布局

```text
┌──────────────────────────────────────────────┐
│                                              │
│                                              │
│                 Atmosphere                   │
│                                              │
│               ┌────────────┐                 │
│               │            │                 │
│               │   Crystal  │                 │
│               │            │                 │
│               └────────────┘                 │
│                                              │
│                    失眠                      │
│                Suki刘舒好                    │
│                                              │
│                    Core                      │
│                                              │
└──────────────────────────────────────────────┘
```

## 10.3 自动行为

```text
进入 Music Space
→ Presence Mode

鼠标移动 / 键盘输入
→ Control Mode

4 秒无交互
→ Presence Mode
```

---

# 11. Control Mode

## 11.1 定义

Control Mode 是临时工具层。

不是另一个页面。

## 11.2 显示内容

- Progress
- Previous / Next
- Volume
- Queue
- Library
- Search
- Keep Moment
- Audio device / MPRIS 状态

## 11.3 Control Mode 布局

```text
┌──────────────────────────────────────────────┐
│ Back       Scene       Keep Moment   Library │
│                                              │
│                 Hero Crystal                 │
│                                              │
│                    失眠                      │
│                Suki刘舒好                    │
│                                              │
│        0:42 ━━━━━━━━━━━━━━━━━━━ 3:31         │
│                                              │
│          Prev      Core      Next            │
│                                              │
│                Volume 72%                    │
│                                              │
│                               Queue / Tracks │
└──────────────────────────────────────────────┘
```

## 11.4 控件浮现规则

- 控件从 Atmosphere 中浮现
- 不允许瞬间出现
- 不使用传统 Toolbar
- 控件透明度低于 Crystal
- 控件消失时缩小、退色、降低深度
- 当前操作结束后恢复安静空间

---

# 12. Hero Crystal UI 方案

## 12.1 Demo 目标

当前 Crystal 仍像封面框，Demo 中必须变成音乐身份主体。

## 12.2 视觉层

```text
Artwork Anchor
+
Color Signature
+
Soft Refraction
+
Edge Bloom
+
Light Weight
+
Optical Gravity
+
Audio Responsive Presence
```

## 12.3 有封面

- 保留封面可识别区域
- 边缘与环境颜色融合
- 不覆盖主体人物或标题
- 轻微立体层和光线偏移
- 不做高强度玻璃特效

## 12.4 无封面

不能只长期显示单个汉字或字母。

Demo 版 Identity Seed：

```text
Title Initial
+
Artist Rhythm
+
Track Color Signature
+
Audio Energy Shape
```

必须明确它是 Aurora 生成身份，不伪装成官方封面。

## 12.5 音频响应

```text
bassEnergy
→ Crystal 呼吸幅度

midEnergy
→ 内部光层密度

highEnergy
→ Edge Bloom

transientEnergy
→ 短暂扩张 / 轻微冲击

audioLevel
→ 总体 Presence
```

响应应克制，不做频谱播放器效果。

---

# 13. Atmosphere UI 方案

## 13.1 Atmosphere 不是背景图

Atmosphere 是完整空间主体。

```text
Atmosphere > UI Chrome
```

## 13.2 视觉语言

关键词：

```text
Human
Emotion
Atmosphere
Memory
Flow
Presence
```

避免：

- 科幻宇宙
- 霓虹赛博
- 复杂星球
- HUD
- 游戏界面
- 强玻璃拟态
- 炫技粒子

风格方向：

- 简约
- 情绪
- 柔和
- 日漫画面感
- 大面积留白 / 暗空间
- 柔和色块
- 慢变化
- 音乐驱动而非随机动画

## 13.3 Atmosphere 参数

```text
Color
Light
Motion
Density
Temperature
Depth
Spatial Behavior
```

Demo 使用现有：

```text
identityColor
audioLevel
bassEnergy
midEnergy
highEnergy
transientEnergy
```

---

# 14. Motion 与 Transition 方案

## 14.1 Motion 原则

```text
Quiet Before Motion
```

动画必须表达状态变化，而不是装饰。

## 14.2 Ambient Behavior

区分：

```text
Motion
= 状态变化

Ambient Behavior
= 存在感
```

Ambient Behavior 包括：

- Idle Drift
- Slow Breathing
- Background Presence
- Low-frequency light movement

## 14.3 Moment → Music Space 标准链路

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

不可丢失：

- Artwork Anchor
- Music Identity
- Emotion Color
- Temporal Trace
- Optical Gravity
- Meaning Context

## 14.4 禁止方案

```text
Moment Fade Out
→
Player Fade In
```

这不能作为最终 Demo 主过渡。

## 14.5 切歌动画

用户确认的方向：

```text
当前 Crystal 放大
→ 向左逐渐变小并消失

下一 Crystal 从右侧出现
→ 从小到大
→ 落位到中心
```

要求：

- 渐进出现
- 渐进消失
- 转盘感
- Apple 系统应用切换的连续感
- 不做瞬间滑页
- 目标感受接近 120Hz 流畅节奏

---

# 15. 设计 Token 建议

## 15.1 Color

```text
Background.Deep
Background.Soft
Text.Primary
Text.Secondary
Text.Muted
Identity.Primary
Identity.Secondary
Memory.Accent
Warm.Accent
```

主题色从歌曲身份派生，但必须经过对比度限制。

## 15.2 Material

```text
Atmosphere Field
Memory Surface
Crystal Surface
Control Surface
Transition Surface
```

避免所有对象都使用同一种玻璃材质。

## 15.3 Typography

建议：

- Ubuntu Sans / 系统字体
- 大标题使用宽松字距
- 歌曲标题不超过 28–32px
- Moment Meaning 使用正常正文，不使用宣传文案字体
- 技术信息只在 Diagnostics 中出现

## 15.4 Spatial

```text
Primary margin: 28–40
Hero safe region: 40%
Control depth: low
Moment optical center: slightly upper
Music Space optical center: true center
```

## 15.5 Motion Timing

Demo 建议：

```text
Soft: 180–240ms
Control Reveal: 260–340ms
Flow: 420–560ms
Track Transition: 700–1100ms
Recall: 900–1400ms
Ambient cycle: 6–14s
```

## 15.6 Easing

优先：

```text
OutCubic
OutQuint
InOutCubic
```

避免：

- Bounce
- Elastic
- 强烈 overshoot
- 高频闪烁

---

# 16. Accessibility 与 Quality

## 16.1 Accessibility

```text
Normal
Reduced Motion
High Contrast
Cognitive Minimal
```

### Reduced Motion

- Stable Anchor
- Cross-dissolve
- 关闭 Refraction
- 关闭持续 Drift
- 保留身份连续性

### Cognitive Minimal

- 隐藏非必要信息
- 简化 Controls
- 禁止连续复杂背景动画

## 16.2 Quality

```text
Eco
Balanced
Immersive
Adaptive
```

### Eco

- 关闭复杂折射
- 降低模糊
- 降低粒子
- 保持 Crystal 可识别

### Balanced

- Demo 默认

### Immersive

- 更完整 Atmosphere
- 更高动态范围
- 高性能设备启用

---

# 17. Demo 验收标准

## 17.1 第一眼

- 截掉标题栏和控制按钮后，画面不能像普通播放器
- Crystal 是第一视觉焦点
- Atmosphere 是空间主体
- 控件不是永久存在

## 17.2 主流程

```text
Home
→ Moment
→ Music Space
→ Presence
→ Control
→ Keep Moment
→ Home
```

完整流程连续运行 10 次，无崩溃。

## 17.3 必须展示

1. Home 真实 Moment
2. Meaning
3. Moment Recall
4. Hero Crystal
5. 音频响应 Atmosphere
6. Presence / Control 自动切换
7. Keep Moment
8. 新 Moment 返回 Home

## 17.4 不应展示

- Component Gallery
- Diagnostics
- 数据库路径
- Missing Source 技术说明
- Local audio fallback 文案
- 调试按钮
- 复杂设置

---

# 18. 两天开发排期

## Day 1 上午

### AUR-DEMO-01

- Presence Mode 状态
- 输入监听
- 4 秒自动退场
- Controls opacity / scale / depth
- Presence / Control 切换测试

## Day 1 下午

### AUR-DEMO-02

- Hero Crystal 放大
- 去卡片感
- Audio Reactive mapping
- Atmosphere 强化
- 无封面 Identity Seed 改良

## Day 1 晚上

### AUR-DEMO-03

- Music Space 视觉收敛
- 1920×1080 检查
- Reduced Motion 快速路径
- 性能检查

## Day 2 上午

### AUR-DEMO-04

- Home 文案
- Home 层级
- Meaning 呈现
- 隐藏 Component Gallery
- Memory 入口降级

## Day 2 下午

### AUR-DEMO-05

- Moment → Crystal → Music Space 精修
- Back 反向转换
- Keep Moment 短反馈
- 切歌过渡检查

## Day 2 晚上

### AUR-DEMO-06

- Release 构建
- 完整流程 10 次
- 修复阻塞问题
- 录制演示视频
- 打 Demo 标签

---

# 19. Demo 后正式路线

## Product Pack A — Music Space

```text
AUR-030
Presence Mode / Control Mode

AUR-031
Hero Crystal / Core / Energy Timeline

AUR-032
Identity-Preserving Transition
```

## Product Pack B — Memory

```text
AUR-033
Moment Lifecycle

AUR-034
Emotional Timeline

AUR-035
Recall Search

AUR-036
Memory Export / Import
```

## Product Pack C — Crystal 与 Intelligence

```text
AUR-037
Real Artwork Identity

AUR-038
Generated Identity Seed

AUR-039
Visual / Performance Gate

AUR-040
Local Music Profile

AUR-041
Emotion Graph
```

## Product Pack D — Atmosphere Intelligence

```text
AUR-042
Minimal Context Model

AUR-043
Memory Association Engine

AUR-044
Atmosphere Semantic Generator
```

## Product Pack E — Ubuntu Integration

```text
AUR-045
Desktop Presence

AUR-046
System Context

AUR-047
Desktop Atmosphere Mode
```

## Product Pack F — Trust 与 Production

```text
AUR-048
Privacy Center

AUR-049
Memory Transparency

AUR-050
Production Data

AUR-051
Accessibility / Performance Gate

AUR-052
Aurora 1.0 Candidate
```

---

# 20. 时间预估

## 两天

可完成：

> 一个第一眼不再像普通播放器、具备完整 Aurora Wow Loop 的 Demo。

## 4–6 周

可完成：

> 可长期试用的 Alpha，Music Space、Memory、Crystal 和 Atmosphere 基本成立。

## 3–5 个月

可完成：

> Beta，本地 Intelligence、Emotion Graph、Context、Desktop Presence 开始成立。

## 6–12 个月

可完成：

> Production 1.0，包含数据迁移、性能、可访问性、Trust、导出、删除和恢复。

---

# 21. 项目最终判断

Aurora 当前不是缺功能。

Aurora 当前缺的是：

```text
视觉主次
空间感
Presence
身份连续性
情绪入口
控制退场
```

两天 Demo 的核心不是增加更多能力，而是让已有能力以正确的 Aurora 方式被看见。

最终 Demo 目标：

> 用户不是在操作一个播放器。  
> 用户是在进入一个由音乐、记忆和氛围共同形成的空间。
