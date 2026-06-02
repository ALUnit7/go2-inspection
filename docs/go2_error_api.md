# GO2 故障服务接口

> 更新时间：2023-10-19

---

## Topic

```
rt/errorlist    类型: String_（JSON 字符串）
```

格式：
```json
{"errors":[[时间戳, 故障源码, 故障码], ...]}
```

---

## 故障源码与故障码

### 底层通信固件（源码 100）
Topic: `rt/lf/lowstate` → `LowState.bit_flag`

| 故障码 | 含义 |
|--------|------|
| `0x80` | 电机通信异常 |
| `0x40` | MCU 通信异常 |
| `0x10` | 电池通信异常 |
| `0x02` | 配电开关异常 |

### 风扇（源码 200）
Topic: `rt/lf/lowstate` → `LowState.fan_frequency[3]`

| 故障码 | 含义 |
|--------|------|
| `0x01` | 左后风扇堵转 |
| `0x02` | 右后风扇堵转 |
| `0x04` | 前置风扇堵转 |

### 电机（源码 300 + leg\*3 + motor）
Topic: `rt/lf/lowstate` → `MotorState[i].reserve[0]`

| 故障码 | 含义 |
|--------|------|
| `0x01` | 过流 |
| `0x04` | 驱动过热 |
| `0x10` | 绕组过热 |
| `0x20` | 编码器异常 |
| `0x100` | 电机通信中断 |

### 雷达（源码 400）
Topic: `rt/utlidar/lidar_state` → `LidarState.error_state`

| 故障码 | 含义 |
|--------|------|
| `0x01` | 电机转速异常 |
| `0x02` | 点云数据异常 |

---

## 比赛中的简单监控

```cpp
// 订阅 rt/lf/lowstate，检查 bit_flag
uint8_t flag = low_state.bit_flag();
if (flag & 0x80) std::cerr << "[WARN] 电机通信异常\n";
if (flag & 0x10) std::cerr << "[WARN] 电池通信异常\n";

// 检查电机温度（12个电机）
for (int i = 0; i < 12; i++) {
    if (low_state.motor_state()[i].temperature() > 70)
        std::cerr << "[WARN] 电机" << i << "过热\n";
}
```
