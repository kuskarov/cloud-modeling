#include "server.h"

#include <typeinfo>

#include "event.h"

void
sim::resources::Server::HandleEvent(const sim::events::Event& event)
{
    try {
        auto& server_event = dynamic_cast<const ServerEvent&>(event);
    } catch (const std::bad_cast& bc) {
        // log error
    }
}

sim::types::RAMBytes
sim::resources::Server::GetRam() const
{
    return ram_;
}

void
sim::resources::Server::SetRam(sim::types::RAMBytes ram)
{
    ram_ = ram;
}

sim::types::CPUHertz
sim::resources::Server::GetClockRate() const
{
    return clock_rate_;
}

void
sim::resources::Server::SetClockRate(sim::types::CPUHertz clock_rate)
{
    clock_rate_ = clock_rate;
}

uint32_t
sim::resources::Server::GetCoreCount() const
{
    return cores_count_;
}

void
sim::resources::Server::SetCoresCount(uint32_t cores_count)
{
    cores_count_ = cores_count;
}
