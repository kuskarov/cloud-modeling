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
    Server() : Resource("Server") {}

    void HandleEvent(const std::shared_ptr<events::Event>& event) override;

    [[nodiscard]] types::RAMBytes GetRam() const;
    void SetRam(types::RAMBytes ram);
    [[nodiscard]] types::CPUHertz GetClockRate() const;
    void SetClockRate(types::CPUHertz clock_rate);
    [[nodiscard]] uint32_t GetCoreCount() const;
    void SetCoresCount(uint32_t cores_count);
    [[nodiscard]] types::Currency GetCost() const;
    void SetCost(types::Currency cost);

 private:
    std::vector<std::shared_ptr<VirtualMachine>> virtual_machines_{};

    types::RAMBytes ram_{};
    types::CPUHertz clock_rate_{};
    uint32_t cores_count_{};
    types::Currency cost_{};
};

}   // namespace sim::resources
