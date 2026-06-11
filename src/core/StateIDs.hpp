#pragma once

enum StateID {
    // ── 主流程 ────────────────────────────────────────────────
    STATE_IDLE              = 0,

    STATE_LINE_FOLLOW_1     = 1,   // 寻迹 → 起点障碍
    STATE_JUMP_START        = 2,   // 跳起点障碍
    STATE_LINE_FOLLOW_2     = 3,   // 寻迹 → 避障区（线消失）
    STATE_OBSTACLE_AVOID    = 4,   // 避障区（UDP导航）
    STATE_LINE_FOLLOW_3     = 5,   // 寻迹 → 台阶（ArUco Tag）
    STATE_CLIMB_STAIRS      = 6,   // L型上下台阶
    STATE_LINE_FOLLOW_4     = 7,   // 寻迹 → 抓取平台
    STATE_GRAB_INITIAL      = 8,   // 识别标识+抓起始物资
    STATE_LINE_FOLLOW_5     = 9,   // 寻迹 → 中转平台（直角弯计数）
    STATE_TRANSFER          = 10,  // 中转平台
    STATE_LINE_FOLLOW_6     = 11,  // 寻迹 → 检测平台（红色圆）
    STATE_DETECTION_POINT   = 12,  // 转身识别警示标志+执行动作
    STATE_LINE_FOLLOW_7     = 13,  // 寻迹 → 放置平台（里程计）
    STATE_PLACE_PLATFORM    = 14,  // 放置场地物资
    STATE_LINE_FOLLOW_8     = 15,  // 寻迹 → 终点障碍
    STATE_JUMP_END          = 16,  // 跳终点障碍
    STATE_RETURN_HOME       = 17,  // 寻迹回启停区

    STATE_DONE              = 99,
};
