#pragma once
#include <string>

// 机械臂通信抽象层（协议待定，当前为桩实现）
class ArmInterface {
public:
    virtual ~ArmInterface() = default;
    virtual bool connect(const std::string& ip, int port) = 0;
    virtual bool grab()    = 0;
    virtual bool release() = 0;
    virtual bool home()    = 0;  // 回零位
};

// 桩实现，用于无机械臂时测试
class ArmStub : public ArmInterface {
public:
    bool connect(const std::string&, int) override { return true; }
    bool grab()    override { return true; }
    bool release() override { return true; }
    bool home()    override { return true; }
};
