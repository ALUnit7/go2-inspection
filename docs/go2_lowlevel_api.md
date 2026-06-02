# GO2 底层服务接口

> 更新时间：2025-11-21  
> ⚠️ 使用底层控制前必须通过 `MotionSwitcherClient.ReleaseMode()` 关闭主运控

---

## Topics

| Topic | 方向 | 内容 |
|-------|------|------|
| `rt/lowcmd` | 发布（写） | 电机控制指令 |
| `rt/lowstate` | 订阅（读） | 电机/IMU/电池/遥控器状态 |

---

## 电机索引顺序

```
FR_0=0  FR_1=1  FR_2=2    // 右前腿：髋外展、髋屈伸、膝
FL_0=3  FL_1=4  FL_2=5    // 左前腿
RR_0=6  RR_1=7  RR_2=8    // 右后腿
RL_0=9  RL_1=10 RL_2=11   // 左后腿
```

---

## LowCmd（控制指令）

```cpp
#include <unitree/idl/go2/LowCmd_.hpp>
#include <unitree/robot/channel/channel_publisher.hpp>

unitree::robot::ChannelPublisher<unitree_go::msg::dds_::LowCmd_> pub("rt/lowcmd");
pub.InitChannel();

unitree_go::msg::dds_::LowCmd_ cmd{};
cmd.head()[0] = 0xFE; cmd.head()[1] = 0xEF;

// 设置单个电机（以右前腿膝关节 FR_2=2 为例）
cmd.motor_cmd()[2].mode() = 0x01;  // FOC工作模式（0x00=待机）
cmd.motor_cmd()[2].q()    = 0.0f;  // 目标位置 rad
cmd.motor_cmd()[2].dq()   = 0.0f;  // 目标速度 rad/s
cmd.motor_cmd()[2].tau()  = 0.0f;  // 目标力矩 N·m
cmd.motor_cmd()[2].kp()   = 0.0f;  // 刚度系数
cmd.motor_cmd()[2].kd()   = 0.0f;  // 阻尼系数

// gpio 控制
// cmd.gpio() &= 0xFE;  // 开启自动充电
// cmd.gpio() |= 0x01;  // 关闭自动充电
// cmd.gpio() &= 0xFD;  // 开启12电机电源
// cmd.gpio() |= 0x02;  // 关闭12电机电源

pub.Write(cmd);
```

---

## LowState（状态读取）

```cpp
#include <unitree/idl/go2/LowState_.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>

unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::LowState_> sub("rt/lowstate");
sub.InitChannel([](const void* msg) {
    auto& s = *(const unitree_go::msg::dds_::LowState_*)msg;

    // IMU
    auto& imu = s.imu_state();
    float roll  = imu.rpy()[0];
    float pitch = imu.rpy()[1];
    float yaw   = imu.rpy()[2];

    // 电机状态（以 FR_2=2 为例）
    float q   = s.motor_state()[2].q();    // 关节位置 rad
    float dq  = s.motor_state()[2].dq();   // 关节速度
    float tau = s.motor_state()[2].tau_est(); // 估计力矩

    // 电池
    uint8_t soc = s.bms_state().soc();     // 电量 1~100%
    int32_t cur = s.bms_state().current(); // 正=充电 负=放电

    // 足端力（0=FR 1=FL 2=RR 3=RL）
    int16_t fr_force = s.foot_force()[0];

    // 组件状态
    uint8_t flag = s.bit_flag();
    bool motor_timeout = flag & 0x80;
    bool remote_timeout = flag & 0x20;
    bool charging = !(flag & 0x04);
});
```

---

## BmsState 电池状态字段

| 字段 | 说明 |
|------|------|
| `soc()` | 电量 1~100% |
| `current()` | 正=充电，负=放电 |
| `status()` | 7=充电中，8=放电中，11=警告 |
| `cycle()` | 充电循环次数 |
| `bq_ntc[2]` | 电池内部温度（BAT1/BAT2） |
| `cell_vol[15]` | 15节单体电压 |

---

## bit_flag 状态位

| 位 | 含义 |
|----|------|
| `&0x80` | 电机超时 |
| `&0x40` | 小MCU超时 |
| `&0x20` | 遥控器超时 |
| `&0x10` | 电池超时 |
| `&0x04` | 自动充电状态（0=充电中） |
| `&0x02` | 板载电流异常 |
| `&0x01` | 运控命令超时 |
