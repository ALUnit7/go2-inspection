# GO2 创建客户应用

> 更新时间：2025-11-21

---

## 目标

创建一个高层应用：让四腿朝天的 Go2 翻身站立，调整姿态后向前行走。

---

## 创建应用文件

```bash
cd ~/unitree_sdk2/example
mkdir user && cd user
touch app_height.cpp
```

---

## 完整示例代码

### app_height.cpp（翻身站立 + 前进）

```cpp
#include <unitree/robot/client/client.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " networkInterface" << std::endl;
    exit(-1);
  }

  unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

  unitree::robot::go2::SportClient sport_client;
  sport_client.SetTimeout(10.0f);
  sport_client.Init();
  sport_client.WaitLeaseApplied();

  int moveTime = 3;
  auto startTime = std::chrono::steady_clock::now();

  while (true)
  {
    sport_client.RecoveryStand();
    sleep(3);
    sport_client.Sit();
    sleep(3);
    sport_client.RiseSit();
    sleep(1);
    sport_client.Move(0.2, 0, 0); // 向前 0.2 m/s
    sleep(2);

    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - startTime);
    if (elapsed.count() >= moveTime) break;
  }

  sleep(2);
  return 0;
}
```

### 姿态控制示例

```cpp
#include <unitree/robot/client/client.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <unistd.h>
#include <cmath>

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " networkInterface" << std::endl;
    exit(-1);
  }

  unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

  unitree::robot::go2::SportClient sport_client;
  sport_client.SetTimeout(10.0f);
  sport_client.Init();
  sport_client.WaitLeaseApplied();

  double t = 0, dt = 0.01;
  t += dt;
  sport_client.Euler(0.4 * sin(2 * t), 0.4 * cos(2 * t) - 0.4, 0.);
  sport_client.BalanceStand();
  usleep(int(dt * 1000000));

  return 0;
}
```

---

## 编译

在 `unitree_sdk2/CMakeLists.txt` 末尾添加：

```cmake
add_executable(app_height example/user/app_height.cpp)
target_link_libraries(app_height unitree_sdk2)
```

然后编译：

```bash
cd ~/unitree_sdk2
mkdir build && cd build
cmake ..
make
```

---

## 运行

> 运行前确保主运控服务（`mcf` / `sport_mode`）处于**正常运行**状态。

```bash
cd ~/unitree_sdk2/build/bin
sudo ./app_height <网卡名称>
```
