#pragma once

#include "actor.h"
#include "event.h"
#include "resource.h"
#include "types.h"
#include "vm.h"

namespace sim::resources {

enum class ServerEventType
{
    kNone,
    kBoot,               // called when the server is Off
    kReboot,             // called when the server is Running
    kBootFinished,       // scheduled by kStartup and kReboot handlers
    kShutdown,           // called when the server is Running
    kShutdownFinished,   // scheduled by kShutdown
    kProvisionVM,
    kKillVM
};

struct ServerEvent : events::Event
{
    ServerEventType type{ServerEventType::kNone};
    std::shared_ptr<VirtualMachine> virtual_machine{};
};

class Server : public Resource
{
 public:
    void HandleEvent(const std::shared_ptr<events::Event>& event) override;

    [[nodiscard]] types::RAMBytes GetRam() const;
    void SetRam(types::RAMBytes ram);
    [[nodiscard]] types::CPUHertz GetClockRate() const;
    void SetClockRate(types::CPUHertz clock_rate);
    [[nodiscard]] uint32_t GetCoreCount() const;
    void SetCoresCount(uint32_t cores_count);

 private:
    enum class ServerState
    {
        kOff = 0,
        kTurningOn = 1,
        kTurningOff = 2,
        kRunning = 3,
        kFailure = 4
    };

    ServerState state_{ServerState::kOff};

    std::vector<std::shared_ptr<VirtualMachine>> virtual_machines_{};

    types::RAMBytes ram_{};
    types::CPUHertz clock_rate_{};
    uint32_t cores_count_{};
};

}   // namespace sim::resources
