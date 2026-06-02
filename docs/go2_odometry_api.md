# GO2 里程计服务接口

> 更新时间：2025-12-02

---

## 一、足式融合里程计

直接从高层状态接口读取，无需额外配置。

```cpp
// 订阅 rt/sportmodestate
auto& s = *(const unitree_go::msg::dds_::SportModeState_*)msg;
float x   = s.position()[0];  // 里程计 x (m)
float y   = s.position()[1];  // 里程计 y (m)
float yaw = s.imu_state().rpy()[2];  // 偏航角 (rad)
```

- 固件 ≥ V1.1.6：参考《运控服务接口 V2.0》
- 固件 < V1.1.6：参考《AI 运动服务接口》

---

## 二、视觉里程计（SVO + D435i）

**环境：** Jetson Orin NX + Ubuntu 20.04 + ROS1 Noetic  
**相机：** RealSense D435i IR1（全局快门单目）  
**算法：** rpg_svo_pro_open（半直接法）  
**项目路径：** `/unitree/module/Odometer_service/`

### 输出 Topic

| Topic | 内容 |
|-------|------|
| `/svo/pose_cam/0` | 相机位姿轨迹（ROS PoseStamped） |

### 启动流程

```bash
# 1. roscore
roscore

# 2. 启动相机（需关闭激光发射器）
roslaunch realsense2_camera rs_camera.launch

# 3. 启动 SVO
cd /unitree/module/Odometer_service/
source devel/setup.bash
roslaunch svo_ros rs_camera.launch

# 4. 适当移动相机完成初始化
```

### 相机配置要点

- 分辨率：640×480，帧率：90fps
- 使用 IR1（红外单目），**关闭激光发射器**
- 标定内参：`fx=fy=384.90, cx=321.48, cy=239.26`，畸变系数全为 0

### 使用注意

- 目标距离 ≥ 1m 较稳定
- 避免超近场景或画面骤变
- 跟丢后会自动重启，频繁跟丢需调整场景或参数
