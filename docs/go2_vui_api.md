# GO2 音量灯光服务接口（VuiClient）

> 更新时间：2025-11-21

---

## VuiClient 类

控制 Go2 的灯光亮度和音量。

**错误码：**

| 错误码 | 含义 |
|--------|------|
| 100 | 传入参数有误 |

---

## 接口列表

### SetBrightness

```cpp
int32_t SetBrightness(int level)
```

设置灯光亮度，`level` 范围 **0~10**（0=关，10=最亮）。

### GetBrightness

```cpp
int32_t GetBrightness(int& level)
```

获取当前灯光亮度。

### SetVolume

```cpp
int32_t SetVolume(int level)
```

设置音量，`level` 范围 **0~10**。

### GetVolume

```cpp
int32_t GetVolume(int& level)
```

获取当前音量。

---

## 注意

- **没有灯光开关接口**，用 `SetBrightness(0)` 关灯，`SetBrightness(10)` 开灯。
- "当心辐射"警示动作（闪烁前灯三次）实现方式：

```cpp
for (int i = 0; i < 3; i++) {
    vc.SetBrightness(10); usleep(300000);  // 亮 0.3s
    vc.SetBrightness(0);  usleep(300000);  // 灭 0.3s
}
vc.SetBrightness(10);  // 恢复常亮
```
