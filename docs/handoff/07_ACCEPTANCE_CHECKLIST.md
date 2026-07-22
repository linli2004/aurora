# Aurora Acceptance Checklist
## General Quality Gate

---

# 1. 每次提交

- [ ] 任务范围明确；
- [ ] 没有无关重构；
- [ ] 构建成功；
- [ ] 相关测试通过；
- [ ] 全部测试通过，或明确说明为什么未运行；
- [ ] Release 可启动；
- [ ] 新行为有日志、截图或视频；
- [ ] 没有新的高等级警告；
- [ ] 文档已同步；
- [ ] Git diff 已人工审查。

---

# 2. Architecture

- [ ] Core / Moment / Crystal / Atmosphere 边界未被破坏；
- [ ] Lifecycle / Experience / Runtime 分离；
- [ ] QML 不直接修改 Domain Data；
- [ ] 公共 API 使用语义属性；
- [ ] 没有字符串状态；
- [ ] 新依赖有 ADR；
- [ ] 数据有 Provenance；
- [ ] Suggested Meaning 未被当作用户事实。

---

# 3. Playback

- [ ] 播放；
- [ ] 暂停；
- [ ] Seek；
- [ ] 音量；
- [ ] Previous；
- [ ] Next；
- [ ] 循环边界；
- [ ] End of Media；
- [ ] 无效文件；
- [ ] 文件被移动；
- [ ] 队列为空；
- [ ] 应用重启恢复；
- [ ] 中文路径；
- [ ] 长文件名；
- [ ] 无 Metadata；
- [ ] 无 Artwork。

---

# 4. Motion

- [ ] Release 构建；
- [ ] 60 Hz；
- [ ] 120 Hz（设备支持时）；
- [ ] 无黑帧；
- [ ] 无最终尺寸跳变；
- [ ] 无图层硬交接；
- [ ] 输入立即响应；
- [ ] 中段连续；
- [ ] 结尾减速自然；
- [ ] 重复点击策略正确；
- [ ] Previous 镜像；
- [ ] 循环队列；
- [ ] Reduced Motion；
- [ ] 窗口缩放；
- [ ] 音频不中断或按设计切换。

---

# 5. Visual Scene

- [ ] Identity Safe Region；
- [ ] 文本可读；
- [ ] Core 可见；
- [ ] Eco；
- [ ] Balanced；
- [ ] Reduced Motion；
- [ ] Pause 后安静；
- [ ] Audio Buffer 不可用 Fallback；
- [ ] Shader Failure Fallback；
- [ ] 无持续满负载；
- [ ] Integrated GPU 基线。

---

# 6. Accessibility

- [ ] Keyboard Only；
- [ ] Focus 可见；
- [ ] Reduced Motion；
- [ ] High Contrast；
- [ ] Text Scale 150%；
- [ ] 长文本不溢出；
- [ ] 颜色不是唯一状态表达；
- [ ] 动画不是唯一信息来源。

---

# 7. Persistence

- [ ] Schema Version；
- [ ] Migration；
- [ ] Round-trip；
- [ ] Corrupt Data；
- [ ] Missing File；
- [ ] Detached；
- [ ] Relink；
- [ ] Delete；
- [ ] Export；
- [ ] Backup；
- [ ] Provenance；
- [ ] User-confirmed data protected。

---

# 8. Production 禁用词

在以下项目未全部通过前，不得使用：

```text
Production Ready
Production Primitive
Final
Archive Locked
```

可以使用：

```text
Prototype
Candidate
Initial Development
Architecture Proven
Alpha
Pending Validation
```
