# GO2 Python SDK 接口

> 更新时间：2025-11-21

---

## 安装

### 依赖
- Python >= 3.8
- cyclonedds == 0.10.2
- numpy
- opencv-python

### 安装步骤

```bash
cd ~
sudo apt install python3-pip
git clone https://github.com/unitreerobotics/unitree_sdk2_python.git
cd unitree_sdk2_python
pip3 install -e .
```

### FAQ：找不到 cyclonedds

报错 `Could not locate cyclonedds`，需先手动编译：

```bash
cd ~
git clone https://github.com/eclipse-cyclonedds/cyclonedds -b releases/0.10.x
cd cyclonedds && mkdir build install && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install -DBUILD_DDSPERF=OFF
cmake --build . --target install

cd ~/unitree_sdk2_python
export CYCLONEDDS_HOME="~/cyclonedds/install"
pip3 install -e .
```

---

## 例程列表

所有例程位于 `example/` 目录，运行前需配置好网络连接（参考《快速开始》）。

### 高层控制

```bash
python3 ./example/go2/high_level/go2_sport_client.py <网卡名>
```

可选测试动作（输入序号执行）：

| ID | 动作 |
|----|------|
| 0 | damp（阻尼） |
| 1 | stand_up |
| 2 | stand_down |
| 3 | move forward |
| 4 | move lateral |
| 5 | move rotate |
| 6 | stop_move |
| 7/8 | switch_gait |
| 9 | balanced stand |
| 10 | recovery |
| 11 | left flip |
| 12 | back flip |
| 13 | free walk |
| 14 | free bound |
| 15 | free avoid |
| 16 | walk stair |
| 17 | walk upright |
| 18 | cross step |
| 19 | free jump |

### 底层电机控制

> 运行前需通过 App 关闭高层运动服务（sport_mode）

```bash
python3 ./example/go2/low_level/go2_stand_example.py <网卡名>
```

### 前置摄像头（OpenCV）

> 需要图形界面，按 ESC 退出

```bash
python3 ./example/go2/front_camera/camera_opencv.py <网卡名>
```

### 遥控器状态获取

```bash
python3 ./example/wireless_controller/wireless_controller.py <网卡名>
```

### 避障开关

```bash
python3 ./example/obstacles_avoid_switch/obstacles_avoid_switch.py <网卡名>
```

### 灯光音量控制

```bash
python3 ./example/vui_client/vui_client_example.py <网卡名>
```

### DDS 通信测试

```bash
python3 ./example/helloworld/publisher.py
# 新终端：
python3 ./example/helloworld/subscriber.py
```
