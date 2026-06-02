# GO2 LiDAR 服务接口

> 更新时间：2024-12-05

---

## Topics 汇总

| Topic | 类型 | 内容 |
|-------|------|------|
| `rt/utlidar/cloud` | `PointCloud2_` | 原始点云（雷达坐标系） |
| `rt/utlidar/cloud_deskewed` | `PointCloud2_` | 去畸变点云（odom 世界坐标系） |
| `rt/utlidar/height_map_array` | `HeightMap_` | 环境高度地图（128×128，0.06m/格） |
| `rt/utlidar/range_info` | `PointStamped_` | **前/左/右障碍物距离** ← 跳跃/避障触发 |
| `rt/utlidar/switch` | `String_` | 雷达开关（`"ON"` / `"OFF"`） |

---

## 1. 障碍物距离（最常用）

**Topic：** `rt/utlidar/range_info`

```cpp
#include <unitree/idl/ros2/PointStamped_.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>

unitree::robot::ChannelSubscriber<geometry_msgs::msg::dds_::PointStamped_> sub("rt/utlidar/range_info");
sub.InitChannel([](const void* msg) {
    auto& m = *(const geometry_msgs::msg::dds_::PointStamped_*)msg;
    float front = m.point().x();  // 前方障碍物距离 m
    float left  = m.point().y();  // 左侧障碍物距离 m
    float right = m.point().z();  // 右侧障碍物距离 m
});
```

> 比赛用途：`front < 阈值` 时触发 `FrontJump()` 跳跃障碍。

---

## 2. 原始点云

**Topic：** `rt/utlidar/cloud`（雷达坐标系）或 `rt/utlidar/cloud_deskewed`（odom 坐标系）

```cpp
#include <unitree/idl/ros2/PointCloud2_.hpp>

unitree::robot::ChannelSubscriber<sensor_msgs::msg::dds_::PointCloud2_> sub("rt/utlidar/cloud");
sub.InitChannel([](const void* msg) {
    auto& m = *(const sensor_msgs::msg::dds_::PointCloud2_*)msg;
    // m.width() = 点数，m.data() = 原始字节流
});
```

---

## 3. 高度地图

**Topic：** `rt/utlidar/height_map_array`  
**注意：** 需要运控程序正常运行才会发布（关闭 mcf 后无数据）。

```cpp
#include <unitree/idl/go2/HeightMap_.hpp>

unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::HeightMap_> sub("rt/utlidar/height_map_array");
sub.InitChannel([](const void* msg) {
    auto& m = *(const unitree_go::msg::dds_::HeightMap_*)msg;
    // 分辨率 m.resolution()，尺寸 m.width() x m.height()
    // 空格值 = 1.0e9，有效高度直接读 m.data()[ix + width*iy]
    // 世界坐标：x = origin[0] + ix*resolution, y = origin[1] + iy*resolution
});
```

---

## 4. 雷达开关

```cpp
#include <unitree/idl/std_msgs/String_.hpp>
#include <unitree/robot/channel/channel_publisher.hpp>

unitree::robot::ChannelPublisher<std_msgs::msg::dds_::String_> pub("rt/utlidar/switch");
pub.InitChannel();

std_msgs::msg::dds_::String_ cmd;
cmd.data() = "OFF";  // 或 "ON"
pub.Write(cmd);
```
