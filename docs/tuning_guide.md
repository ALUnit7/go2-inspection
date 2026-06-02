# 寻迹调参指南

---

## 第一步：图像质量（运行 t9，不需要机器人）

| 参数 | 滑动条 | 目标 |
|------|--------|------|
| 曝光 | `Exp(x100us)` | 线条清晰，不过曝不欠曝 |
| 增益 | `Gain x10` | 噪点多时降低 |
| 阈值 | `Thresh` | `3-Binary` 中黑线变白、背景全黑 |
| 闭运算 | `CloseK` | `4-Morphed` 中白色内部黑点消失（当前值：12） |
| 开运算 | `OpenK` | `4-Morphed` 中白色外部小噪点消失（当前值：5） |

目标：`4-Morphed` 中线条是干净的白色实心块，背景是纯黑。

---

## 第二步：检测参数（t9，对着实际赛道）

| 参数 | 滑动条 | 目标 |
|------|--------|------|
| 远处采样行 | `TopRow` | 直道稳定显示 `TOP:OK`，弯道允许 `TOP:LOST` |

验证符号：
- 线右倾 → `angle` 应为**负**
- 线在右侧 → `offset` 应为**负**

---

## 第三步：控制增益（t10，连机器人，先低速 SPEED=0.1）

**解算逻辑：**
```
vyaw = angle  × KP    → yaw 转向（追线方向）
vy   = offset × KP2   → 侧向平移（横向归中）
Move(SPEED, vy, vyaw)
```

| 参数 | 调法 | 现象 |
|------|------|------|
| `KP` | 直道轻微倾斜，观察纠正平滑度 | 太大：来回摆动；太小：纠偏慢 |
| `KP2` | 机器人整体偏侧，观察平移归中 | 太大：过冲；太小：偏侧不纠正 |

---

## 第四步：速度（直道稳定后）

| 参数 | 调法 |
|------|------|
| `SPEED` | 从 0.1 逐步提速，观察控制是否跟得上 |
| 弯道减速系数 | t10 代码中 `SPEED * 0.3f`，弯道不稳时调小 |

---

## 当前参数记录

```cpp
// t10_line_follow_robot.cpp 调参区
THRESH      = 67
CLOSE_K     = 12
OPEN_K      = 5
KP          = 0.008f
KP2         = 0.001f
SPEED       = 0.25f
EXPOSURE_US = -1      // 自动
GAIN        = -1      // 自动
```

---

## 弯道行为

| 状态 | 速度 | 转向 | 终端标记 |
|------|------|------|---------|
| `top_ok=true` | SPEED | angle×KP | `[OK]` |
| `top_ok=false` | SPEED×0.3 | 上一帧 vyaw（惯性） | `[TURN]` |
| `valid=false` >30帧 | 停止 | 0 | `Line lost!` |

---

## 日志分析

```bash
# 查看最新日志
ls -t /tmp/line_follow_*.csv | head -1 | xargs cat | head -50

# 找丢线时刻（valid=0）
grep -n ",0," /tmp/line_follow_*.csv

# 找弯道模式时刻（top_ok=0）
grep -n ",[01],[0]$" /tmp/line_follow_*.csv
```

日志列：`time_ms, angle, offset, vy, vyaw, valid, top_ok`
