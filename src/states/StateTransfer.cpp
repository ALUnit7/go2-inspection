#include "StateTransfer.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <iostream>
void StateTransfer::enter(RobotContext& ctx) {
    ticks_=0; ctx.sport->stop();
    std::cout << "[Transfer] ARM: unload + grab field item (placeholder)\n";
}
int StateTransfer::update(RobotContext& ctx) {
    if (++ticks_ > 150) return STATE_LINE_FOLLOW_6;
    return STATE_STAY;
}
