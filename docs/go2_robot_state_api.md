# GO2 设备状态服务接口（RobotStateClient）

> 更新时间：2026-02-26

---

## 接口列表

### ServiceSwitch

```cpp
int32_t ServiceSwitch(const std::string& name, int32_t swit, int32_t& status)
```

| 参数 | 说明 |
|------|------|
| name | 服务名（见下表） |
| swit | 1=开启，0=关闭 |
| status | 执行后状态（0=已开启，1=已关闭） |

**错误码：** 5201=执行错误，5202=服务受保护不可操作

### ServiceList

```cpp
int32_t ServiceList(std::vector<ServiceState>& serviceStateList)
```

查询所有服务当前状态。每项包含 `name`、`status`、`protect`。

### SetReportFreq

```cpp
int32_t SetReportFreq(int32_t interval, int32_t duration)
```

设置服务状态上报频率（供 App 使用）。

---

## 服务名列表

| 服务名 | 描述 | 备注 |
|--------|------|------|
| `mcf` | 主运动控制服务 | 固件 ≥ V1.1.6 |
| `sport_mode` | 主运动控制服务 | 固件 < V1.1.6 |
| `basic_service` | 底层服务 | |
| `obstacles_avoid` | 避障服务 | |
| `video_hub` | 视频源服务 | |
| `unitree_lidar` | 雷达服务 | |
| `vui_service` | 离线语音服务 | |
| `utrack` | 伴随服务 | |
| `webrtc_bridge` | WebRTC 通信服务 | |
| `net_switcher` | 网络开关 | |
| `ota_box` | OTA 固件升级服务 | |
| `chat_go` | 笨笨狗服务 | |

---

## 典型用法

```cpp
#include <unitree/robot/go2/robot_state/robot_state_client.hpp>

unitree::robot::ChannelFactory::Instance()->Init(0, "enp108s0");
unitree::robot::go2::RobotStateClient rsc;
rsc.SetTimeout(10.f); rsc.Init();

// 查询所有服务状态
std::vector<unitree::robot::go2::ServiceState> list;
rsc.ServiceList(list);
for (auto& s : list)
    std::cout << s.name << " status=" << s.status << " protect=" << s.protect << "\n";

// 关闭主运控（低层控制前必须执行）
int32_t status;
rsc.ServiceSwitch("mcf", 0, status);

// 重新开启
rsc.ServiceSwitch("mcf", 1, status);
```
