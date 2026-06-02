# GO2 运控服务接口 V2.0

> 更新时间：2025-11-21 | 适用版本：Go2 Edu ≥ V1.1.6

---

## 版本说明

- 发布日期：2025-05-12，正式版本 V1.1.6
- 调用前请更新 `unitree_sdk2` 到最新版本
- 低于 V1.1.6 请参考旧版文档（运控切换服务接口 / 高层运动服务接口 / AI 运动服务接口）

---

## 接口概览

| 接口类型 | 实现方式 | 功能 |
|----------|----------|------|
| 高层控制接口 | `sport_client` | 模式切换、速度控制等运动指令 |
| 高层状态接口 | 订阅 `rt/sportmodestate` | 位置、速度、姿态、运动模式 |

---

## 高层控制接口

### 初始化示例

```cpp
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <unistd.h>

int main(int argc, char **argv)
{
  unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]); // argv[1] 为网卡名称

  unitree::robot::go2::SportClient sport_client;
  sport_client.SetTimeout(10.0f);
  sport_client.Init();

  sport_client.Sit();
  sleep(3);
  sport_client.RiseSit();
  return 0;
}
```

### 控制函数列表

#### 基础状态控制

| 函数 | 原型 | 功能 | 备注 |
|------|------|------|------|
| `Damp` | `int32_t Damp()` | 进入阻尼状态（急停） | 最高优先级，所有电机停止 |
| `BalanceStand` | `int32_t BalanceStand()` | 平衡站立 | 机身姿态和高度始终保持平衡，可配合 `Euler()` 使用 |
| `StopMove` | `int32_t StopMove()` | 停止当前动作 | 恢复内部运动参数为默认值 |
| `StandUp` | `int32_t StandUp()` | 关节锁定，站高 | 默认高度 0.33m，长时间锁定易过热 |
| `StandDown` | `int32_t StandDown()` | 关节锁定，趴下 | |
| `RecoveryStand` | `int32_t RecoveryStand()` | 恢复站立 | 从翻倒或趴下状态恢复 |

#### 运动控制

| 函数 | 原型 | 功能 | 参数范围 |
|------|------|------|----------|
| `Euler` | `int32_t Euler(float roll, float pitch, float yaw)` | 姿态控制 | roll/pitch: [-0.75, 0.75] rad；yaw: [-0.6, 0.6] rad |
| `Move` | `int32_t Move(float vx, float vy, float vyaw)` | 速度控制 | vx: [-2.5, 3.8] m/s；vy: [-1.0, 1.0] m/s；vyaw: [-4, 4] rad/s |
| `SpeedLevel` | `int32_t SpeedLevel(int level)` | 速度档位 | -1 慢速，0 正常，1 快速 |

> `Move` 注意：运控不对指令滤波，最新指令维持 1s。建议自行加滤波；不使用时发送 `Move(0,0,0)` 或 `StopMove()`。

#### 步态模式

| 函数 | 功能 | 备注 |
|------|------|------|
| `FreeWalk()` | 灵动模式（默认步态） | AI 灵动，支持爬楼梯、碎石、草甸、湿滑地面 |
| `ClassicWalk(bool)` | 经典步态 | AI 经典，行走姿态稳定优雅 |
| `StaticWalk()` | 常规行走 | 不具备复杂地形能力，行走优雅 |
| `TrotRun()` | 常规跑步 | 最高 3.7m/s，不具备复杂地形能力 |
| `EconomicGait()` | 续航模式 | 机身较高，单块长续航电池可达 ~4h |
| `FreeBound(bool)` | 并腿跑模式 | bound 步态 |
| `FreeJump(bool)` | 跳跃模式 | 跳跃奔跑步态 |
| `FreeAvoid(bool)` | 闪避模式 | 移动时避障，静止时对前方物体闪避 |
| `WalkUpright(bool)` | 后腿直立模式 | 电机易过热，注意控制时长 |
| `CrossStep(bool)` | 交叉步模式 | 电机易过热，注意控制时长 |
| `HandStand(bool)` | 倒立行走 | 需先调用 `BalanceStand()`，电机易过热 |

#### 特殊动作

| 函数 | 功能 | 备注 |
|------|------|------|
| `Sit()` | 坐下 | 需等上一动作完成后执行 |
| `RiseSit()` | 站起（从坐下恢复） | |
| `Hello()` | 打招呼 | |
| `Stretch()` | 伸懒腰 | |
| `Content()` | 开心 | |
| `Heart()` | 比心 | |
| `Pose(bool)` | 摆姿势 | true 摆姿势，false 恢复 |
| `Scrape()` | 拜年作揖 | |
| `Dance1()` | 舞蹈段落 1 | |
| `Dance2()` | 舞蹈段落 2 | |

#### 高危动作（注意安全距离）

| 函数 | 功能 | 备注 |
|------|------|------|
| `FrontFlip()` | 前空翻 | 危险，可能加速硬件损伤 |
| `FrontJump()` | 前跳 | |
| `FrontPounce()` | 向前扑人 | |
| `LeftFlip()` | 左空翻 | 需在非运动状态，结束后自动进入灵动 |
| `BackFlip()` | 后空翻 | 需在非运动状态，结束后自动进入灵动 |

#### 其他配置

| 函数 | 功能 | 备注 |
|------|------|------|
| `SwitchJoystick(bool)` | 遥控器响应开关 | false 时推摇杆不干涉程序 |
| `AutoRecoverSet(bool)` | 设置自动翻身 | 有背载设备时建议关闭 |
| `AutoRecoverGet(bool&)` | 查询自动翻身状态 | |
| `SwitchAvoidMode()` | 闪避模式下关闭静止时的障碍物闪避 | 一般不建议使用 |

### 错误码

| 错误码 | 描述 |
|--------|------|
| 4101 | 轨迹点数错误（客户端返回） |
| 4201 | 动作超时（未在期望时间内完成） |
| 4205 | 状态机未初始化结束 |
| 4206 | 执行挥手/拜年类动作前姿态不佳 |
| 3104 | DDS 超时 |

### 姿态控制示例

```cpp
#include <cmath>
#include <signal.h>
#include <unistd.h>
#include <unitree/robot/go2/sport/sport_client.hpp>

bool stopped = false;
void sigint_handler(int sig) { if (sig == SIGINT) stopped = true; }

int main(int argc, char **argv)
{
  unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

  unitree::robot::go2::SportClient sport_client;
  sport_client.SetTimeout(10.0f);
  sport_client.Init();

  double t = 0, dt = 0.01;
  signal(SIGINT, sigint_handler);

  while (!stopped)
  {
    t += dt;
    sport_client.Euler(0.2 * sin(2 * t), 0.2 * cos(2 * t) - 0.2, 0.);
    sport_client.BalanceStand();
    usleep(int(dt * 1000000));
  }

  sport_client.Euler(0, 0, 0);
  return 0;
}
```

---

## 高层状态接口

### 订阅示例

```cpp
#include <unitree/idl/go2/SportModeState_.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>

#define TOPIC_HIGHSTATE "rt/sportmodestate"

void HighStateHandler(const void* message)
{
  auto state = *(unitree_go::msg::dds_::SportModeState_*)message;
  std::cout << "position: "
            << state.position()[0] << ", "
            << state.position()[1] << ", "
            << state.position()[2] << std::endl;
  std::cout << "quaternion(w,x,y,z): "
            << state.imu_state().quaternion()[0] << ", "
            << state.imu_state().quaternion()[1] << ", "
            << state.imu_state().quaternion()[2] << ", "
            << state.imu_state().quaternion()[3] << std::endl;
}

int main()
{
  unitree::robot::ChannelFactory::Instance()->Init(0, "enp2s0");
  unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_> suber(TOPIC_HIGHSTATE);
  suber.InitChannel(HighStateHandler);
  while(1) usleep(20000);
  return 0;
}
```

### 状态字段

```cpp
TimeSpec stamp();
uint32_t error_code();          // 当前运动状态机编号
IMU imu_state();                // IMU 状态
std::array<float, 3> position(); // 三维位置
float body_height();            // 机体高度
std::array<float, 3> velocity(); // 三维速度
float yaw_speed();              // 偏航速度
```

**IMU 数据：**

```cpp
std::array<float, 4> quaternion();    // 四元数 (w,x,y,z)
std::array<float, 3> gyroscope();     // 角速度 (rad/s)
std::array<float, 3> accelerometer(); // 加速度 (m/s²)
std::array<float, 3> rpy();           // 欧拉角 (rad)
int8_t temperature();                 // 温度
```

### error_code 状态机对照表

| error_code | 状态 |
|------------|------|
| 100 | 灵动 |
| 1001 | 阻尼 |
| 1002 | 站立锁定 |
| 1004 / 2006 | 蹲下 |
| 1006 | 打招呼 / 伸懒腰 / 舞蹈 / 拜年 / 比心 / 开心 |
| 1007 | 坐下 |
| 1008 | 前跳 |
| 1009 | 扑人 |
| 1013 | 平衡站立 |
| 1015 | 常规行走 |
| 1016 | 常规跑步 |
| 1017 | 常规续航 |
| 1091 | 摆姿势 |
| 2007 | 闪避 |
| 2008 | 并腿跑 |
| 2009 | 跳跃跑 |
| 2010 | 经典 |
| 2011 | 倒立 |
| 2012 | 前空翻 |
| 2013 | 后空翻 |
| 2014 | 左空翻 |
| 2016 | 交叉步 |
| 2017 | 直立 |
| 2019 | 牵引 |
