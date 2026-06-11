#include "StatePlacePlatform.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <iostream>
void StatePlacePlatform::enter(RobotContext& ctx) {
    ticks_=0; ctx.sport->stop();
    std::cout << "[PlacePlatform] marker_id=" << ctx.memory.marker_id
              << " ARM: place to platform " << ctx.memory.marker_id << " (placeholder)\n";
}
int StatePlacePlatform::update(RobotContext& ctx) {
    if (++ticks_ > 150) return STATE_LINE_FOLLOW_8;
    return STATE_STAY;
}
