# GO2 UWB 服务接口

> 更新时间：2025-01-02  
> ⚠️ 本项目不使用 UWB（无基站），仅供参考

---

## Topic

| Topic | 内容 |
|-------|------|
| `rt/uwbstate` | UWB 定位数据（需要 UWB 基站硬件） |

---

## 主要字段（UwbState_）

```cpp
// 标签在基站坐标系中的位置（球坐标）
float orientation_est;  // 方位角 rad
float pitch_est;        // 俯仰角 rad
float distance_est;     // 距离 m
float yaw_est;          // 标签朝向偏航角 rad

// 标签 IMU
float tag_roll, tag_pitch, tag_yaw;

// 机身 IMU
float base_roll, base_pitch, base_yaw;

// 摇杆（伴随遥控器）
float joystick[2];  // [0]=x(上正) [1]=y(左正)，范围 -1~1
```
