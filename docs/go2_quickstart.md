# GO2 SDK 快速开始

> 更新时间：2025-11-21

---

## 环境要求

| 项目 | 要求 |
|------|------|
| 操作系统 | Ubuntu 20.04 或 22.04（不支持 Mac/Windows，不支持 Go2 内置电脑） |
| 网络 | 用户电脑网卡设置在 `192.168.123.x` 网段，建议 `192.168.123.222`，**禁止使用 `.161`** |

### 依赖安装

```bash
sudo apt install cmake gcc build-essential libeigen3-dev
```

---

## 安装 SDK

```bash
cd ~/unitree_sdk2/
mkdir build && cd build
cmake ..
sudo make install
```

安装到指定目录：

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/unitree_robotics
sudo make install
```

---

## 编译例程

```bash
cd ~/unitree_sdk2
mkdir build && cd build
cmake ..
make
```

编译成功后，二进制文件位于 `build/bin/` 目录下。

---

## 配置网络

1. 用网线连接 Go2 与用户电脑，开启 USB Ethernet
2. 将网卡 IP 设置为 `192.168.123.222`（Go2 内置电脑 IP 为 `192.168.123.161`）
3. 验证连接：

```bash
ping 192.168.123.161
```

4. 查看网卡名称（运行例程时需要）：

```bash
ifconfig
# 找到 IP 为 192.168.123.222 对应的网卡名，如 enxf8e43b808e06
```

---

## 运行例程

### 底层控制例程（low_level）

> 底层例程会控制 Go2 右前腿规律摆动，**运行前需让机器人四腿悬空**。

**Step 1：关闭主运控服务**

通过 App → 设备 → 服务状态 关闭，或通过 `go2_stand_example` 中的运控切换服务接口关闭。

运控服务名称对照：

| 服务名称 | 说明 |
|----------|------|
| `mcf` | Go2 主运控服务（软件版本 > 1.1.6） |
| `sport_mode` | Go2 主运控服务（软件版本 < 1.1.6） |
| `wheeled_sport` | Go2 W 主运控服务 |

> 必须关闭主运控服务，否则多个运控同时发送指令会导致机器狗失控。

**Step 2：运行**

```bash
cd ~/unitree_sdk2/build/bin
./go2_low_level <网卡名称>
# 例：./go2_low_level enxf8e43b808e06
```

---

### 站立例程（go2_stand_example）

控制机器人从趴下 → 站立 → 趴下，**会自动关闭主运控服务，无需手动关闭**。

> 运行前确保 Go2 处于静止趴地状态。

```bash
cd ~/unitree_sdk2/build/bin
./go2_stand_example <网卡名称>
```
