# GO2 AI 运动服务接口

> 更新时间：2025-11-21 | 适用：Go2 EDU，固件 < V1.1.6  
> 固件 ≥ V1.1.6 请参考《运控服务接口 V2.0》  
> 使用前需将运控切换至 AI 模式（App 或 MotionSwitcherClient）

---

## 与高层运动接口的主要差异

| 项目 | AI 模式 | 高层模式 |
|------|---------|---------|
| `Move` vx 范围 | [-0.6, 0.6] m/s | [-2.5, 3.8] m/s |
| `Move` vy 范围 | [-0.4, 0.4] m/s | [-1.0, 1.0] m/s |
| `Move` vyaw 范围 | [-0.8, 0.8] rad/s | [-4, 4] rad/s |
| `SwitchMoveMode` | ✅ 有 | ❌ 无 |
| `WalkStair` | ✅ 有 | ✅ 有 |

---

## 新增接口

### SwitchMoveMode

```cpp
int32_t SwitchMoveMode(bool flag)
```

切换 `Move()` 响应模式：
- `true`：持续响应模式，一直执行最新 Move 指令
- `false`（默认）：未收到新指令时延迟 1s 后自动停止

> ⚠️ 默认关闭，建议谨慎开启。

---

## 里程计 Topic

```cpp
// 高频 500Hz
#define TOPIC_HIGHSTATE  "rt/sportmodestate"

// 低频（里程计，用于 MoveToPos）
#define TOPIC_ODOM_STATE "rt/lf/odommodestate"
```

`MoveToPos` 配合 `rt/lf/odommodestate` 使用，订阅当前位置后计算目标坐标。

---

## MoveToPos 用法

```cpp
// 订阅里程计获取当前位姿，计算目标点，再调用
sport_client.MoveToPos(target_x, target_y, target_yaw);
```

参考坐标系为里程计坐标系（机器人启动时的位置为原点）。

---

## 接口列表（与高层运动接口相同部分略）

完整接口见《高层运动服务接口》，以下仅列 AI 模式特有或有差异的：

| 函数 | AI模式备注 |
|------|-----------|
| `Move(vx,vy,vyaw)` | 速度范围更小，更保守 |
| `SwitchMoveMode(bool)` | AI 模式专有 |
| `RecoveryStand()` | 仅翻倒时响应（高层模式任何时候都响应） |
| `FrontFlip()` | 结束后进入盲走模式（高层模式进入灵动） |
