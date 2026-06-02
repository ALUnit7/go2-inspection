# GO2 高层运动服务接口（SportClient）

> 更新时间：2025-11-21 | 适用：Go2 EDU，固件 < V1.1.6  
> 固件 ≥ V1.1.6 请参考《运控服务接口 V2.0》

---

## 头文件

```cpp
#include <unitree/robot/go2/sport/sport_client.hpp>
```

---

## 控制接口

### 基础状态

| 函数 | 功能 | 备注 |
|------|------|------|
| `Damp()` | 急停，所有电机进入阻尼 | 最高优先级 |
| `BalanceStand()` | 平衡站立 | 可配合 `Euler()` / `BodyHeight()` |
| `StopMove()` | 停止并重置运动参数 | |
| `StandUp()` | 关节锁定站高（默认 0.33m） | 长时间易过热 |
| `StandDown()` | 关节锁定趴下 | |
| `RecoveryStand()` | 从翻倒/趴下恢复站立 | |

### 运动控制

| 函数 | 参数范围 | 功能 |
|------|----------|------|
| `Move(vx,vy,vyaw)` | vx[-2.5,3.8] vy[-1,1] vyaw[-4,4] | 速度控制（机体坐标系） |
| `Euler(roll,pitch,yaw)` | roll/pitch[-0.75,0.75] yaw[-0.6,0.6] rad | 姿态控制 |
| `BodyHeight(height)` | [-0.18, 0.03] m（相对默认 0.33m） | 机身高度 |
| `FootRaiseHeight(height)` | [-0.06, 0.03] m（相对默认 0.09m） | 抬足高度 |
| `SpeedLevel(level)` | -1=慢 0=正常 1=快 | 速度档位 |
| `MoveToPos(x,y,yaw)` | 里程计坐标系 | 移动到绝对位置 |
| `TrajectoryFollow(path)` | 30个PathPoint | 轨迹跟踪 |
| `ContinuousGait(flag)` | bool | 持续踏步（速度为0时也保持） |

### 步态模式

| 函数 | 功能 | 备注 |
|------|------|------|
| `FreeWalk()` | 灵动模式（默认） | AI步态，复杂地形 |
| `WalkStair(bool)` | **爬楼梯模式** | true进入，false退出回灵动 |
| `SwitchGait(d)` | 切换步态 | 0=idle 1=trot 2=trot run **3=正向爬楼** **4=逆向爬楼** |
| `FreeAvoid(bool)` | 闪避模式 | 移动时避障，静止时闪避前方物体 |
| `FreeBound(bool)` | 并腿跑 | |
| `FreeJump(bool)` | 跳跃模式 | |
| `WalkUpright(bool)` | 后腿直立 | 易过热 |
| `CrossStep(bool)` | 交叉步 | 易过热 |
| `HandStand(bool)` | 倒立行走 | 需先 BalanceStand |

### 特殊动作

| 函数 | 功能 |
|------|------|
| `Sit()` | 坐下 |
| `RiseSit()` | 从坐下站起 |
| `Hello()` | 打招呼 ← **当心强氧化物** |
| `Stretch()` | 伸懒腰 ← **当心触电** |
| `Wallow()` | 打滚 |
| `Pose(bool)` | 摆姿势 |
| `Scrape()` | 拜年作揖 |
| `Dance1()` / `Dance2()` | 舞蹈 |
| `FrontFlip()` | 前空翻 |
| `FrontJump()` | **前跳** ← 跳跃障碍 |
| `FrontPounce()` | 向前扑人 |
| `LeftFlip()` | 左空翻（非运动状态） |
| `BackFlip()` | 后空翻（非运动状态） |

### 其他

| 函数 | 功能 |
|------|------|
| `SwitchJoystick(bool)` | 遥控器响应开关（false=程序独占） |
| `GetState(vector, map)` | 获取当前运动状态 |

---

## 错误码

| 错误码 | 描述 |
|--------|------|
| 4101 | 轨迹点数错误 |
| 4201 | 动作超时 |
| 4205 | 状态机未初始化 |
| 4206 | 执行挥手/拜年前姿态不佳 |
| 3104 | DDS 超时 |

---

## 高层状态接口

订阅 topic：`rt/sportmodestate`

```cpp
#include <unitree/idl/go2/SportModeState_.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>

unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_> sub("rt/sportmodestate");
sub.InitChannel([](const void* msg) {
    auto& s = *(unitree_go::msg::dds_::SportModeState_*)msg;
    // 位置、速度、姿态...
});
```

### 状态字段

```cpp
std::array<float, 3> position();          // 三维位置 (x,y,z)
std::array<float, 3> velocity();          // 三维速度
float bodyHeight();                        // 机体高度
float yawSpeed();                          // 偏航速度
std::array<float, 4> rangeObstacle();     // 四方向障碍物距离 ← 避障/跳跃触发
std::array<int16_t, 4> footForce();       // 四足端力
uint32_t errorCode();                      // 运动状态机编号
uint8_t gaitType();                        // 步态类型
```

### IMU 数据

```cpp
std::array<float, 4> quaternion();   // 四元数 (w,x,y,z)
std::array<float, 3> gyroscope();    // 角速度 rad/s
std::array<float, 3> accelerometer();// 加速度 m/s²
std::array<float, 3> rpy();          // 欧拉角 rad ← pitch 判断台阶
int8_t temperature();
```

### gaitType 枚举

| 值 | 步态 |
|----|------|
| 0 | idle |
| 1 | trot |
| 2 | run |
| 3 | climb stair |
| 4 | forwardDownStair |
| 9 | adjust |

---

## 比赛关键接口速查

| 任务 | 推荐接口 |
|------|----------|
| 跳跃障碍 | `FrontJump()` |
| 避障区通过 | `FreeAvoid(true)` + `ObstaclesAvoidClient` |
| 台阶通过 | `WalkStair(true)` 或 `FreeWalk()` + 监控 `rpy()[1]`(pitch) |
| 检测平台停靠 | `StopMove()` + `BalanceStand()` |
| 当心触电 | `Stretch()` |
| 当心强氧化物 | `Hello()` |
| 当心辐射 | `VuiClient::SetBrightness` 闪烁 |
| 障碍物距离感知 | `rangeObstacle()` |
