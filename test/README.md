# GO2 SDK 验证测试

连接机器人后逐项运行，均需 `sudo`。

```bash
# CLion 直接 Build，或命令行：
cd /home/wzl/GO2_RCOM/build && make -j$(nproc)
```

---

## 程序说明

### t1_connection — 连接验证 ⭐ 第一个跑
订阅 `rt/sportmodestate`，持续打印位置/姿态/速度/error_code。  
**用途：** 确认 SDK 与机器人通信正常。

```bash
sudo ./t1_connection enp108s0
```

---

### t2_basic_motion — 基础运动
FreeWalk → 前进 2s → 停止 → 后退 2s → 停止。  
**用途：** 验证 `Move` / `StopMove` / `FreeWalk` 基本调用。

```bash
sudo ./t2_basic_motion enp108s0
```

---

### t3_jump — 跳跃障碍
BalanceStand → FrontJump → 监控 error_code 判断落地。  
**用途：** 验证跳跃动作，观察落地时 error_code 变化。  
⚠️ 前方至少留 1m 空间。

```bash
sudo ./t3_jump enp108s0
```

---

### t4_obstacle_avoid — 避障
依次测试两种方式：
1. `FreeAvoid(true)` + `Move` 前进 5s
2. `ObstaclesAvoidClient` + `UseRemoteCommandFromApi(true)` + `Move`

**用途：** 验证避障区通过方案，确认 `UseRemoteCommandFromApi` 必须先调用。

```bash
sudo ./t4_obstacle_avoid enp108s0
```

---

### t5_climb_stairs — 台阶通过
FreeWalk 低速前进，每 100ms 打印 pitch 角。  
**用途：** 标定台阶通过时 pitch 的实际变化范围，用于设置 `StateClimbStairs` 的退出阈值。

```bash
sudo ./t5_climb_stairs enp108s0
```

---

### t6_actions — 检测平台三个动作
验证警示标志对应动作：

| 参数 | 警示标志 | 动作 | SDK 调用 |
|------|---------|------|---------|
| `0` | 当心触电 | 伸懒腰 | `Stretch()` |
| `1` | 当心强氧化物 | 打招呼 | `Hello()` |
| `2` | 当心辐射 | 提示用 t7 | — |

```bash
sudo ./t6_actions enp108s0 0
sudo ./t6_actions enp108s0 1
```

---

### t7_vui_light — 前灯闪烁三次
`SetBrightness(10/0)` 循环三次，对应"当心辐射"动作。  
**用途：** 验证 VuiClient 灯光控制（注意：无 SetSwitch，只有 SetBrightness）。

```bash
sudo ./t7_vui_light enp108s0
```

---

### t8_camera — 摄像头读图
`VideoClient.GetImageSample` 读取单帧，保存到 `/tmp/go2_frame.jpg`。  
**用途：** 验证 SDK 直接读图，查看图像质量。

```bash
sudo ./t8_camera enp108s0
eog /tmp/go2_frame.jpg   # 查看图片
```

---

## 建议测试顺序

```
t1（连通）→ t8（摄像头）→ t7（灯光）→ t6（动作）→ t2（运动）→ t4（避障）→ t5（台阶）→ t3（跳跃）
```
