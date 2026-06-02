# GO2 多模态巡检系统

基于 `unitree_sdk2` (C++) 的 GO2 比赛二次开发框架。

## 目录结构

```
src/
├── main.cpp                  # 入口
├── core/                     # 状态机驱动、共享上下文
│   ├── MissionRunner         # 顶层状态机循环
│   ├── StateBase             # 状态基类
│   ├── StateIDs              # 状态枚举
│   └── RobotContext          # 共享上下文（SDK客户端+配置）
├── states/                   # 比赛任务状态（每个任务一个文件）
│   ├── StateIdle             # 等待启动
│   ├── StateLineFollow       # 寻迹
│   ├── StateJump             # 跳跃障碍
│   ├── StateObstacleAvoid    # 避障
│   ├── StateClimbStairs      # 攀爬台阶
│   ├── StateVisionTask       # 视觉识别
│   └── StateArmControl       # 机械臂抓取
├── robot/                    # SDK 封装层
│   ├── SportController       # SportClient 封装
│   └── StateMonitor          # rt/sportmodestate 订阅
├── perception/               # 感知层
│   ├── CameraClient          # VideoClient → cv::Mat
│   ├── Detector              # YOLO 推理（OpenCV DNN）
│   └── DetectionResult       # 检测结果数据结构
└── arm/
    └── ArmInterface          # 机械臂抽象层（协议待定）
```

## 任务流程

```
IDLE → LINE_FOLLOW → JUMP → LINE_FOLLOW → OBSTACLE_AVOID
     → CLIMB_STAIRS → VISION_TASK → ARM_CONTROL → DONE
```

## 构建

```bash
# 依赖：unitree_sdk2（已安装）、OpenCV、yaml-cpp
./scripts/build.sh
```

## 运行

```bash
# 本地（开发调试）
sudo ./build/go2_inspection config/mission.yaml

# 部署到 Jetson 并运行
./scripts/deploy_jetson.sh
ssh unitree@192.168.123.18 "cd go2_inspection && sudo ./build/go2_inspection"
```

## 配置

编辑 `config/mission.yaml`：
- `network.interface`：连接 Go2 的网卡名
- `tasks.*`：各任务速度、阈值参数

## 添加新状态

1. 在 `src/core/StateIDs.hpp` 添加枚举值
2. 在 `src/states/` 新建 `StateXxx.hpp/.cpp`，继承 `StateBase`
3. 在 `src/core/MissionRunner.cpp` 的 `register_states()` 注册
4. 在相关状态的 `update()` 中返回新状态 ID
