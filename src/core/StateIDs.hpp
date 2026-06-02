#pragma once

enum StateID {
    STATE_IDLE           = 0,
    STATE_LINE_FOLLOW    = 1,
    STATE_JUMP           = 2,
    STATE_OBSTACLE_AVOID = 3,
    STATE_CLIMB_STAIRS   = 4,
    STATE_VISION_TASK    = 5,
    STATE_ARM_CONTROL    = 6,
    STATE_DONE           = 99,
};
