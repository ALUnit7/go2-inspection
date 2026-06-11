#pragma once

// 跨状态传递的比赛关键信息
struct MissionMemory {
    int  marker_id      = 0;    // 抓取平台标识：1=1号平台，2=2号平台
    int  warning_sign   = -1;   // 警示标志：0=辐射，1=触电，2=强氧化物
    int  corner_count   = 0;    // LINE_FOLLOW_5 中直角弯计数
    float total_dist_m  = 0.f;  // 从启动开始的里程计累积距离(m)
    bool initial_grabbed= false;
    bool field_grabbed  = false;
};
