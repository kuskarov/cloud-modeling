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
    kStartup,        // called when the server is Off
    kReboot,         // called when the server is Running
    kBootFinished,   // scheduled by kStartup and kReboot handlers
    kShutdown,       // called when the server is Running
    kProvisionVM,
    kKillVM
};

struct ServerEvent : public events::Event
{
    ServerEventType type{ServerEventType::kNone};
    std::optional<VirtualMachine> virtual_machine{};
};

class Server : public Resource
{
 public:
    void HandleEvent(const events::Event& event) override;

    void ProvisionVM() {}

    [[nodiscard]] types::RAMBytes GetRam() const;
    void SetRam(types::RAMBytes ram);
    [[nodiscard]] types::CPUHertz GetClockRate() const;
    void SetClockRate(types::CPUHertz clock_rate);
    [[nodiscard]] uint32_t GetCoreCount() const;
    void SetCoresCount(uint32_t cores_count);

 private:
    enum class ServerState
    {
        kOff,
        kTurningOn,
        kTurningOff,
        kRunning,
        kFailure
    };

    ServerState state_{ServerState::kOff};

    types::RAMBytes ram_{};
    types::CPUHertz clock_rate_{};
    uint32_t cores_count_{};
};

}   // namespace sim::resources
