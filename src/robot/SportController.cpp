#include "SportController.hpp"
#include <iostream>

void SportController::init(float timeout) {
    // ChannelFactory 已在 main 中初始化，此处只初始化 client
    client_.SetTimeout(timeout);
    client_.Init();
}

bool SportController::log_error(const char* fn, int code) {
    if (code != 0)
        std::cerr << "[Sport] " << fn << " error=" << code << "\n";
    return code == 0;
}

int SportController::move(float vx, float vy, float vyaw) {
    int r = client_.Move(vx, vy, vyaw);
    log_error("Move", r);
    return r;
}
int SportController::stop()          { return client_.StopMove(); }
int SportController::balanceStand()  { return client_.BalanceStand(); }
int SportController::recoveryStand() { return client_.RecoveryStand(); }
int SportController::freeWalk()      { return client_.FreeWalk(); }
int SportController::walkStair(bool e) { return e ? client_.FreeWalk() : 0; }  // SDK无WalkStair，FreeWalk支持台阶
int SportController::jump()          { return client_.FrontJump(); }
int SportController::hello()         { return client_.Hello(); }
int SportController::stretch()       { return client_.Stretch(); }
int SportController::sit()           { return client_.Sit(); }
int SportController::riseSit()       { return client_.RiseSit(); }
int SportController::switchJoystick(bool e) { return client_.SwitchJoystick(e); }
