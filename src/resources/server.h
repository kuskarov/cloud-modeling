#pragma once

#include "actor.h"
#include "event.h"
#include "resource.h"
#include "types.h"

namespace sim::resources {

class VirtualMachine
{
};

enum class ServerEventType
{
    kNone,
    kStartup,
    kReboot,
    kShutdown,
    kProvisionVM,
    kKillVM,
    kMigrate,
};

struct ServerEvent : public events::Event
{
    ServerEventType type{};
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
    types::RAMBytes ram_{};
    types::CPUHertz clock_rate_{};
    uint32_t cores_count_{};
};

}   // namespace sim::resources
