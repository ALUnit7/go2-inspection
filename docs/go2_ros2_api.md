# GO2 ROS2 服务接口

> 更新时间：2026-04-01  
> 仓库：https://github.com/unitreerobotics/unitree_ros2

---

## ⚠️ 重要冲突说明

**unitree_sdk2 与 ROS2 不能在同一进程中使用**，两者都占用 CycloneDDS。  
在拓展坞 Terminal 中，启动时**不要 source ROS 环境**（直接回车跳过）。

---

## 关键 Topics（ROS2 下无 `rt/` 前缀）

| ROS2 Topic | 对应 DDS Topic | 内容 |
|------------|---------------|------|
| `/sportmodestate` | `rt/sportmodestate` | 高层运动状态（高频） |
| `/lf/sportmodestate` | `rt/lf/sportmodestate` | 高层运动状态（低频） |
| `/lowstate` | `rt/lowstate` | 底层状态 |
| `/wirelesscontroller` | `rt/wirelesscontroller` | 遥控器摇杆/按键 |
| `/api/sport/request` | — | 高层运动控制指令 |
| `/lowcmd` | `rt/lowcmd` | 底层电机控制 |
| `/utlidar/cloud` | `rt/utlidar/cloud` | 雷达点云 |

---

## 遥控器状态（本项目可用于调试触发）

```cpp
// Topic: /wirelesscontroller
float lx, ly;   // 左摇杆
float rx, ry;   // 右摇杆
uint16 keys;    // 按键键值
```

---

## 环境配置（Jetson 上）

```bash
# setup.sh 关键内容
source /opt/ros/foxy/setup.bash
source ~/unitree_ros2/cyclonedds_ws/install/setup.bash
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces>
    <NetworkInterface name="enp108s0" .../></Interfaces></General></Domain></CycloneDDS>'
```

> 本项目使用 unitree_sdk2（C++）开发，**不使用 ROS2**。  
> ROS2 仅用于调试可视化（rviz2 查看点云）或遥控器状态读取。
