# 主决策状态机设计

> 持续更新，每次讨论后同步

---

## 比赛完整序列

```
IDLE
 │ 手动触发
 ▼
LINE_FOLLOW_1        沿黑线到起点障碍
 │ ObstacleDetector 触发（白色障碍物遮断黑线，ROI内白色像素骤减）
 ▼
JUMP_START           FrontJump()，计时等待落地（LAND_WAIT_MS）
 │ 落地稳定后恢复 FreeWalk
 ▼
LINE_FOLLOW_2        沿黑线到避障区
 │ valid=false 持续N帧 且 ObstacleDetector 未触发 → 线消失非障碍物
 ▼
OBSTACLE_AVOID       避障区通过
 │ 接收 UDP 127.0.0.1:9876 导航速度指令 {vx,vy,vyaw}
 │ ObstaclesAvoidClient.Move 执行（未接入时 FreeAvoid+直行占位）
 │ valid=true 恢复稳定（黑线重新出现）→ 关闭避障
 ▼
LINE_FOLLOW_3        沿黑线到台阶
 │ ① ArucoTag 检测到台阶 Tag（前置摄像头）→ 减速，继续直行
 │ ② 图像上半部分白色占满（台阶正面全黑→反色全白）→ 停止
 ▼
CLIMB_STAIRS         L型路径上下三级台阶（内部4个phase）
 │ APPROACH：极低速直行贴近台阶
 │ CLIMB_UP：FreeWalk低速上台阶，pitch升高确认上台，pitch回稳到顶停
 │ TURN_LEFT：原地左转90°（IMU yaw累计计数）
 │ CLIMB_DOWN：FreeWalk低速下台阶，pitch下倾确认，pitch回稳完成
 ▼
LINE_FOLLOW_4        沿黑线到抓取平台
 │ 里程计区间内 + 雷达左距（range_info.y）突变
 ▼
GRAB_INITIAL         抓取起始物资 + 识别1/2号标识
 │ ① 左转90°（前置摄像头正对平台）
 │ ② SignDetector 识别标识 → 写入 marker_id
 │ ③ 右转90°回正（侧对平台）
 │ ④ vy 向左侧移到平台旁
 │ ⑤ 机械臂抓取起始物资【待定】
 │ ⑥ vy 向右侧移回赛道
 ▼
LINE_FOLLOW_5        沿黑线到中转平台
 │ 里程计区间内 + 第2次直角弯通过后固定距离
 │ （第1次直角弯=台阶侧面后的转角，第2次直角弯=中转平台前的转角）
 │ 直角弯检测：top_ok=false + 纯yaw控制转过85°→corner_count++
 ▼
TRANSFER             中转平台：卸载起始物资 + 抓场地物资【待定，机械臂】
 ▼
LINE_FOLLOW_6        沿黑线到检测平台
 │ 相机检测到地面红色圆形（HSV红色 + 圆形轮廓）
 ▼
DETECTION_POINT      检测平台完整流程
 │ ① 停止
 │ ② 左转90°
 │ ③ YOLO 识别警示标志（最多3s，取最高置信度）
 │ ④ 执行动作：辐射→灯光闪烁三次 / 触电→Stretch() / 强氧化物→Hello()
 │ ⑤ 右转90°回原朝向
 ▼
LINE_FOLLOW_7        沿黑线到放置平台
 │ 里程计区间到达放置区（两个平台夹着寻迹线，左1右2，固定存在）
 ▼
PLACE_PLATFORM       放置场地物资
 │ marker_id=1 → 机械臂向左伸出放置到1号平台
 │ marker_id=2 → 机械臂向右伸出放置到2号平台
 │ 无需侧移，两平台夹着赛道【待定，机械臂】
 ▼
LINE_FOLLOW_8        沿黑线到终点障碍
 │ ObstacleDetector 触发
 ▼
JUMP_END             FrontJump()，计时等待落地
 ▼
RETURN_HOME          沿黑线回启停区
 │ 里程计/计时停靠【待定】
 ▼
DONE
```

---

## 直角弯检测与计数

```
触发条件：top_ok=false（前方看不到线）
控制切换：停止寻迹，切换为纯yaw控制（固定 vyaw 转弯）
完成条件：IMU yaw 累计变化 > 85° → corner_count++ → 切回寻迹
```

LINE_FOLLOW_5 退出条件：`corner_count == 2` 后行驶固定距离停下
（第1次弯=台阶侧面后转角，第2次弯=中转平台前转角，第2次弯通过后即到中转平台）

---

## 平台位置与检测方式

| 平台 | 位置 | 到位检测 | 操作 |
|------|------|---------|------|
| 抓取平台 | 线左侧 | 里程计区间+雷达左距突变 | 左转识别→回正侧移→抓取→回线 |
| 中转平台 | 线右侧 | 第2次直角弯后固定距离 | 机械臂卸载+抓取（无需侧移） |
| 检测平台 | 线上红色圆 | 相机检测红色圆形 | 左转→YOLO→动作→右转 |
| 放置平台 | 线两侧（左1右2） | 里程计到达放置区 | 机械臂左/右伸出放置（无需侧移） |

---

## DETECTION_POINT 详细流程

```
LINE_FOLLOW_6 寻迹中：
  海康相机检测到地面红色圆 → 继续寻迹（不立即停）
  红色圆消失（圆到了机身正下方）→ 停止
  （相机在机器狗前侧，检测到圆时圆还在前方，需等圆到机身中间再停）

停止后：
  ① 左转90°（IMU yaw 计数 85°）
  ② GO2 前置摄像头：YOLO 识别警示标志（最多3s，取最高置信度）
  ③ 执行动作：辐射→灯光闪烁三次 / 触电→Stretch() / 强氧化物→Hello()
  ④ 右转90°（yaw 计数 85°）回原朝向
```

**注意：** 红色圆检测用海康相机（朝下看地面），YOLO 用 GO2 前置摄像头（转身后朝向标志牌）。

**双相机说明：**
- 海康相机：寻迹 + 障碍物检测 + 红色圆检测（所有地面感知）
- GO2 前置摄像头：ArUco Tag + YOLO 警示标志 + 1/2号标识（所有标志感知）
- RobotContext 需同时持有两个相机（待实现）



```cpp
struct MissionMemory {
    int  marker_id    = 0;   // 1或2，GRAB_INITIAL写，PLACE_PLATFORM读
    int  warning_sign = 0;   // 0=辐射 1=触电 2=强氧化物
    int  corner_count = 0;   // 直角弯计数，LINE_FOLLOW_5使用
    bool initial_grabbed = false;
    bool field_grabbed   = false;
};
```

---

## 状态触发条件汇总

| 状态 | 进入条件 | 退出条件 | 确认 |
|------|---------|---------|------|
| IDLE | 程序启动 | 手动触发 | ✅ |
| LINE_FOLLOW_1 | IDLE完成 | ObstacleDetector触发 | ✅ |
| JUMP_START | 障碍检测 | 计时落地 | ✅ |
| LINE_FOLLOW_2 | 跳跃落地 | valid=false N帧+无障碍 | ✅ |
| OBSTACLE_AVOID | 线消失 | valid=true恢复 | ✅ |
| LINE_FOLLOW_3 | 避障完成 | ArUco Tag检测+全黑停止 | ✅ |
| CLIMB_STAIRS | 台阶触发 | 4个phase完成 | ✅ |
| LINE_FOLLOW_4 | 台阶完成 | 里程计+雷达左距突变 | ✅ |
| GRAB_INITIAL | 到位 | 机械臂完成 | ❓ |
| LINE_FOLLOW_5 | 抓取完成 | corner_count==2+固定距离 | ✅ |
| TRANSFER | 到位 | 机械臂完成 | ❓ |
| LINE_FOLLOW_6 | 中转完成 | 红色圆形检测 | ✅ |
| DETECTION_POINT | 红色圆触发 | 动作完成+右转回正 | ✅ |
| LINE_FOLLOW_7 | 检测完成 | 里程计到放置区 | ✅ |
| PLACE_PLATFORM | 到位 | 机械臂完成 | ❓ |
| LINE_FOLLOW_8 | 放置完成 | ObstacleDetector触发 | ✅ |
| JUMP_END | 障碍检测 | 计时落地 | ✅ |
| RETURN_HOME | 跳跃落地 | 里程计/计时 | ❓ |
| DONE | 停靠完成 | — | ✅ |

---

## 待实现的 test

| Test | 功能 | 状态 |
|------|------|------|
| t14 | 障碍物检测调试（视觉ROI） | ✅ |
| t15 | 障碍物检测+跳跃 | ✅ |
| t16 | 地面红色圆检测 | ❌ |
| t17 | 雷达侧距平台到位调试 | ❌ |
| t18 | 台阶：ArUco+全黑检测+爬台阶 | ❌ |
| t19 | DETECTION_POINT完整流程 | ❌ |
| t20 | 直角弯检测+计数 | ❌ |

---

## 避障区进程间通信

```
协议：UDP 127.0.0.1:9876，12字节 float32×3（vx/vy/vyaw）
方向：ROS2导航进程 → SDK进程
超时：>200ms未收到 → Move(0,0,0)
当前占位：FreeAvoid(true) + Move(SPEED,0,0)
```

---

## 待讨论

1. 停靠区判断（RETURN_HOME退出条件）
2. 机械臂协议
3. 里程计各区间具体数值（需现场测量）
