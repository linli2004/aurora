# Aurora Architecture Guardrails
## Frozen Boundaries for Local Development

---

# 1. Source of Truth

文档优先级：

1. `docs/specifications/Aurora_Initial_Development_Baseline_0.1.md`
2. 组件 Specification
3. Product Definition
4. Engineering Documents
5. Task Spec
6. 临时实现说明

低优先级文件不得静默覆盖高优先级边界。

出现冲突时：

- 停止扩大修改；
- 记录冲突；
- 提交用户与 ChatGPT 评审；
- 在 `docs/decisions/` 建立 ADR。

---

# 2. Primitive 冻结职责

```text
Core
=
Runtime State

Moment
=
Memory Identity

Crystal
=
Music Identity

Atmosphere
=
Environment
```

## Aurora Core

负责：

- 播放和运行状态的可感知表达；
- Idle、Gathering、Playing、Paused、Transition、Recall 等语义状态；
- 接收用户操作意图。

不负责：

- 专辑封面身份；
- 用户记忆意义；
- 音乐库数据；
- 持久化。

## Aurora Moment

负责：

- 音乐体验记忆身份；
- 时间、环境、用户确认意义；
- Recall 入口；
- Provenance。

不负责：

- 实时播放状态；
- 视觉歌曲身份；
- 音频分析；
- 完整 Music Space。

## Aurora Crystal

负责：

- 音乐身份；
- Artwork / Generated Identity；
- 跨 Home、Moment、Transition、Music Space 的识别连续性。

不负责：

- 用户人生意义；
- 播放状态；
- Memory Lifecycle。

## Aurora Atmosphere

负责：

- 光、色温、空间、运动和环境；
- 音频或 Emotion 参数的可感知表达；
- Scene 渲染。

不负责：

- Track 身份；
- Moment 持久化；
- 用户事实。

---

# 3. 三层状态必须分离

```text
Lifecycle State
≠
Experience State
≠
Runtime State
```

- Lifecycle：对象经历了什么；
- Experience State：用户感受到什么；
- Runtime State：程序正在做什么。

禁止：

- 用数据库状态直接控制 QML Variant；
- 用字符串拼写状态；
- 把 Loading、Playing、Remembered 混在同一枚举；
- 让视觉组件自行推断业务生命周期。

---

# 4. Data / Presentation / Intent 分离

```text
Domain Data
≠
QML Presentation State
≠
User Intent
```

要求：

- Domain Data 从 QML 看应为只读；
- 用户操作通过 signal / command 表达；
- C++ Service 或 Resolver 修改状态；
- QML 不直接写数据库；
- C++ 不写视觉常量；
- 公共 API 使用语义属性。

允许：

```qml
experienceState
presenceLevel
qualityMode
accessibilityMode
transitionPersonality
```

禁止公开：

```qml
glowRadius
blurAmount
refractionStrength
particleCount
rawColorValue
```

---

# 5. Recognition Before Decoration

任何效果关闭后，音乐身份仍必须可识别。

Eco、Reduced Motion、Shader Failure 时仍需保留：

- Crystal 识别；
- Track Title；
- Artist；
- 播放状态；
- 操作可达性。

性能降级应先移除：

- 粒子；
- 折射；
- 持续漂移；
- 高成本 Blur；
- 多层 Shader。

不得先移除身份和可读性。

---

# 6. Meaning Belongs to Human

AI 或算法可以生成：

- Suggested Meaning；
- Emotion Estimate；
- Moment Candidate；
- Atmosphere Parameters。

不得自动写成：

- 用户事实；
- 用户确认意义；
- 人生事件描述；
- 永久 Memory。

必须区分：

```text
System Suggested
User Accepted
User Authored
```

---

# 7. Local First 与 Provenance

默认：

```text
Device
→
Local Processing
→
Optional Sync
```

任何自动生成的数据至少记录：

- source；
- processingLocation；
- generatedBy；
- modelVersion；
- userConfirmed；
- lastUpdated；
- delete / modify path。

---

# 8. 变更必须先写 ADR 的情况

- 新增第三方依赖；
- 修改 Primitive 职责；
- 新数据库或 Schema；
- 新音频后端；
- 新 Provider 协议；
- 新线程模型；
- 新 Shader 编译方式；
- 改变 Qt 最低版本；
- 改变数据所有权；
- 改变隐私默认值；
- 改变 Transition Engine 公共接口。

---

# 9. 停止线

Codex 遇到以下情况必须停止并汇报：

- 规范冲突；
- 测试无法运行；
- 需要删除用户数据；
- 需要联网下载未知依赖；
- 需要逆向或绕过音乐服务限制；
- 需要把 Suggested Meaning 当作用户事实；
- 需要跨 Primitive 移动职责；
- 需要大规模重构才能完成一个小任务；
- Release 动画与 Debug 结果明显不同但原因不明。
