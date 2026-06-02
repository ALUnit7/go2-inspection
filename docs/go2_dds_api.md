# GO2 DDS 通信接口

> 更新时间：2025-09-24

---

## 概述

`unitree_sdk2` 是对 DDS 的封装层，提供：
- DDS 组件的 QoS 配置
- 基于 DDS Topic 的发布/订阅通信
- 基于 DDS Topic 的 RPC 请求/响应机制

适用于 Go2 内部进程间及 Go2 外部与内部的进程间通信。

---

## ChannelFactory

单例工厂，使用前必须初始化。

```cpp
unitree::robot::ChannelFactory::Instance()->Init(0);
```

### 接口列表

| 函数 | 原型 | 说明 |
|------|------|------|
| `Instance` | `static ChannelFactory* Instance()` | 获取单例指针 |
| `Init` | `void Init(int32_t domainId, const std::string& networkInterface="", bool enableSharedMemory=false)` | 指定 domainId、网卡名、共享内存初始化 |
| `Init` | `void Init(const std::string& ddsParameterFileName="")` | 从 JSON 配置文件初始化 |
| `Init` | `void Init(const JsonMap& param)` | 从 JsonMap 配置初始化 |
| `CreateSendChannel` | `template<MSG> ChannelPtr CreateSendChannel(const std::string& name)` | 创建发送 Channel |
| `CreateRecvChannel` | `template<MSG> ChannelPtr CreateRecvChannel(const std::string& name, callback, int32_t queuelen=0)` | 创建接收 Channel |
| `Release` | `void Release()` | 释放静态资源 |

**注意：**
- `networkInterface` 为空时自动选择网卡
- Go2 外部开发时 `enableSharedMemory` 必须为 `false`
- 配置文件不存在或网卡不可用时抛出异常

---

## ChannelPublisher

发布指定类型消息。

> 构造前必须先初始化 `ChannelFactory`。

```cpp
template<typename MSG>
class ChannelPublisher {
  explicit ChannelPublisher(const std::string& channelName);
  void InitChannel();
  void CloseChannel();
  bool Write(const MSG& msg); // true=成功，false=失败
};
```

---

## ChannelSubscriber

订阅指定类型消息。

> 构造前必须先初始化 `ChannelFactory`。

```cpp
template<typename MSG>
class ChannelSubscriber {
  explicit ChannelSubscriber(const std::string& channelName);
  void InitChannel(const std::function<void(const void*)>& callback, int64_t queuelen=0);
  void CloseChannel();
  int64_t GetLastDataAvailableTime(); // 未初始化返回-1，否则返回微秒时间戳
};
```

> 回调处理耗时较长时，建议设置 `queuelen > 0` 避免 DDS 回调线程阻塞。

---

## Service Client 通用接口

Go2 内部服务组件通过 RPC Client 对外提供接口（如 `SportClient`、`RobotStateClient`、`VuiClient`、`ObstaclesAvoidClient`）。

### 通用方法

| 函数 | 说明 |
|------|------|
| `void Init()` | 客户端初始化，完成 API 注册 |
| `void SetTimeout(float timeout)` | 设置 RPC 超时（秒），默认 1s |
| `void WaitLeaseApplied()` | 申请租约（阻塞），仅启用租约时有效 |
| `const std::string& GetApiVersion()` | 获取客户端 API 版本 |
| `const std::string& GetServerApiVersion()` | 获取服务端 API 版本 |

### 通用错误码

| 错误码 | 描述 | 来源 |
|--------|------|------|
| 3001 | 未知错误 | 客户端/服务端 |
| 3102 | 请求发送错误 | 客户端 |
| 3103 | API 未注册 | 客户端 |
| 3104 | 请求超时 | 客户端 |
| 3105 | 请求与响应数据不匹配 | 客户端 |
| 3106 | 响应数据无效 | 客户端 |
| 3107 | 租约无效 | 客户端 |
| 3201 | 响应发送错误 | 服务端（不返回客户端） |
| 3202 | 服务端内部错误 | 服务端 |
| 3203 | API 在服务端未实现 | 服务端 |
| 3204 | API 参数错误 | 服务端 |
| 3205 | 请求被拒绝 | 服务端 |
| 3206 | 租约无效 | 服务端 |
| 3207 | 租约已存在 | 服务端 |
