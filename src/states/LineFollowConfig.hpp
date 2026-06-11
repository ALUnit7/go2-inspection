#pragma once

struct LineFollowConfig {
    // 退出条件（可多选，取先触发者）
    bool exit_on_obstacle   = false;  // 白色障碍物遮断黑线
    bool exit_on_line_lost  = false;  // 黑线消失（非障碍，进入避障区）
    bool exit_on_tag        = false;  // 检测到指定 ArUco Tag
    bool exit_on_corner     = false;  // 直角弯计数达到目标值
    bool exit_on_odometry   = false;  // 里程计+雷达侧距
    bool exit_on_red_circle = false;  // 检测到地面红色圆形

    int   target_tag_id   = -1;   // exit_on_tag 时的目标 ID
    int   target_corners  = 0;    // exit_on_corner 时的目标弯道数
    float target_distance = 0.f;  // exit_on_odometry 时的里程计目标(m)
    char  radar_side      = 'N';  // 'L'=左距突变, 'R'=右距突变, 'N'=仅里程计

    int next_state = 0;
};
