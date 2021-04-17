#include "server.h"

#include <loguru.hpp>
#include <typeinfo>

#include "event.h"

void
sim::resources::Server::HandleEvent(const sim::events::Event& event)
{
    try {
        auto& server_event = dynamic_cast<const ServerEvent&>(event);

        switch (server_event.type) {
            case ServerEventType::kStartup: {
                // switch state to kTurningOn and schedule kBootFinished event
                if (state_ != ServerState::kOff) {
                    LOG_F(ERROR,
                          "Startup event received, but server is not in Off "
                          "state");
                    state_ = ServerState::kFailure;
                    break;
                }

                state_ = ServerState::kTurningOn;

                ServerEvent boot_finished_event{};
                boot_finished_event.type = ServerEventType::kBootFinished;
                boot_finished_event.happen_ts =
                    server_event.happen_ts + startup_delay_;

                schedule_callback_(boot_finished_event.happen_ts,
                                   std::move(boot_finished_event), false);
                break;
            }
            case ServerEventType::kReboot: {
            }
            default: {
                LOG_F(ERROR,
                      "Server %s received server event with invalid type",
                      name_.c_str());
                break;
            }
        }

    } catch (const std::bad_cast& bc) {
        LOG_F(ERROR, "Server %s received non-server event!", name_.c_str());
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
