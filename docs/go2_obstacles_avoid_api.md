# GO2 避障服务接口

> 更新时间：2025-11-21

---

## ObstaclesAvoidClient 类

`ObstaclesAvoidClient` 是 Go2 避障服务提供的 Client，通过 RPC 方式实现对 Go2 避障功能的开关控制。

---

## 接口列表

### SwitchSet

```cpp
int32_t SwitchSet(bool enable)
```

设置避障功能开启或关闭。`enable=true` 开启，`false` 关闭。

### SwitchGet

```cpp
int32_t SwitchGet(bool& enable)
```

获取避障功能当前状态。

### UseRemoteCommandFromApi

```cpp
int32_t UseRemoteCommandFromApi(bool isRemoteCommandsFromApi)
```

抢夺遥控器的速度指令控制权。**必须设为 `true`，才能通过 API 控制避障移动。**

### Move

```cpp
int32_t Move(float x, float y, float yaw)
```

避障模式下速度控制。

| 参数 | 范围 | 说明 |
|------|------|------|
| x | [-1.5, 1.5] m/s | 机身 x 方向速度 |
| y | [-1.0, 1.0] m/s | 机身 y 方向速度 |
| yaw | [-1.57, 1.57] rad/s | 偏航角速度 |

### MoveToIncrementPosition

```cpp
int32_t MoveToIncrementPosition(float x, float y, float yaw)
```

避障模式下增量位置控制。

| 参数 | 范围 | 说明 |
|------|------|------|
| x | [-2.0, 2.0] m | x 方向位置增量 |
| y | [-2.0, 2.0] m | y 方向位置增量 |
| yaw | [-1.57, 1.57] rad | yaw 角增量 |

### MoveToAbsolutePosition

```cpp
int32_t MoveToAbsolutePosition(float x, float y, float yaw)
```

避障模式下绝对位置控制（世界坐标系）。

---

## 使用流程

```
SwitchSet(true)              // 1. 开启避障
UseRemoteCommandFromApi(true) // 2. 抢夺控制权（必须）
Move(vx, vy, vyaw)           // 3. 发送速度指令，自动绕障
...
UseRemoteCommandFromApi(false) // 4. 释放控制权
SwitchSet(false)              // 5. 关闭避障
```

---

## 例程

```cpp
#include <unitree/robot/go2/obstacles_avoid/obstacles_avoid_client.hpp>

int main()
{
    unitree::robot::ChannelFactory::Instance()->Init(0, "enp108s0");

    unitree::robot::go2::ObstaclesAvoidClient sc;
    sc.SetTimeout(5.0f);
    sc.Init();

    sc.SwitchSet(true);
    usleep(1000000);
    sc.UseRemoteCommandFromApi(true);
    sc.Move(1.0, 0.0, 0.0);  // x方向 1m/s，碰到障碍物自动避障

    int t = 0;
    while (true) {
        usleep(1000000);
        if (++t > 5) {
            sc.UseRemoteCommandFromApi(false);
            sc.SwitchSet(false);
            break;
        }
    }
    return 0;
}
```
