# GO2 运控切换服务接口（MotionSwitcherClient）

> 更新时间：2025-11-21 | 适用：Go2 EDU

---

## 运控模式名

| 服务名 | 适用固件版本 |
|--------|-------------|
| `mcf` | ≥ V1.1.6 |
| `ai` / `normal` / `advanced` | < V1.1.6 |

---

## 接口列表

### CheckMode

```cpp
int32_t CheckMode(std::string& form, std::string& name)
```

检测当前形态和运控模式。

| 输出参数 | 说明 |
|----------|------|
| form | `"0"` = 标准形态，`"1"` = 轮足形态 |
| name | 当前运控模式名 |

### SelectMode

```cpp
int32_t SelectMode(const std::string& name)
```

切换到指定运控模式（如 `"mcf"`）。

### ReleaseMode

```cpp
int32_t ReleaseMode()
```

释放当前运控模式（切换低层控制前调用）。

### SetSilent / GetSilent

```cpp
int32_t SetSilent(bool silent)
int32_t GetSilent(bool& silent)
```

静默模式：Go2 重启后默认不启动任何运控服务。

---

## 错误码

| 错误码 | 描述 |
|--------|------|
| 7001 | 请求参数错误 |
| 7002 | 切换服务繁忙，稍后重试 |
| 7004 | 运控模式名不支持 |
| 7005~7008 | 内部指令执行错误 |
| 7009 | 自定义配置错误 |

---

## 头文件

```cpp
#include <unitree/comm/motion_switcher/motion_switcher_client.hpp>
```

---

## 典型用法

```cpp
unitree::robot::ChannelFactory::Instance()->Init(0, "enp108s0");
unitree::comm::MotionSwitcherClient msc;
msc.SetTimeout(5.f); msc.Init();

// 查询当前模式
std::string form, name;
msc.CheckMode(form, name);
std::cout << "form=" << form << " mode=" << name << "\n";

// 释放运控（进入低层控制前）
msc.ReleaseMode();

// 恢复运控
msc.SelectMode("mcf");
```

---

## 与 RobotStateClient 的区别

| | MotionSwitcherClient | RobotStateClient.ServiceSwitch |
|-|----------------------|-------------------------------|
| 用途 | 专用于运控模式切换 | 通用服务开关 |
| 推荐场景 | 切换高层/低层控制 | 开关避障、雷达等其他服务 |
| 查询当前模式 | ✅ CheckMode | ❌ |
