#include "server.h"

#include <loguru.hpp>
#include <typeinfo>

#include "event.h"

void
sim::resources::Server::HandleEvent(const std::shared_ptr<events::Event>& event)
{
    try {
        auto server_event = dynamic_cast<const ServerEvent*>(event.get());

        switch (server_event->type) {
            case ServerEventType::kBoot: {
                // switch state to kTurningOn and schedule kBootFinished event

                if (state_ != ServerState::kOff) {
                    LOG_F(ERROR,
                          "Boot event received, but server is not in Off "
                          "state");
                    state_ = ServerState::kFailure;
                    break;
                }

                state_ = ServerState::kTurningOn;

                auto boot_finished_event = std::make_shared<ServerEvent>();
                boot_finished_event->addressee = this;
                boot_finished_event->type = ServerEventType::kBootFinished;
                boot_finished_event->happen_ts =
                    server_event->happen_ts + startup_delay_;

                schedule_callback_(boot_finished_event->happen_ts,
                                   boot_finished_event, false);

                LOG_F(INFO, "Server Boot event raised");

                break;
            }
            case ServerEventType::kReboot: {
                break;
            }
            case ServerEventType::kBootFinished: {
                // switch state to kRunning

                if (state_ != ServerState::kTurningOn) {
                    LOG_F(ERROR,
                          "BootFinished event received, but server is not in "
                          "TurningOn state");
                    state_ = ServerState::kFailure;
                    break;
                }

                LOG_F(INFO, "BootFinished Boot event raised");

                state_ = ServerState::kRunning;

                break;
            }
            case ServerEventType::kShutdown: {
                // switch state to kTurningOff and schedule kShutdownFinished
                // event

                if (state_ != ServerState::kRunning) {
                    LOG_F(ERROR,
                          "Shutdown event received, but server is not in "
                          "Running state");
                    state_ = ServerState::kFailure;
                    break;
                }

                state_ = ServerState::kTurningOff;

                auto shutdown_finished_event = std::make_shared<ServerEvent>();
                shutdown_finished_event->addressee = this;
                shutdown_finished_event->type =
                    ServerEventType::kShutdownFinished;
                shutdown_finished_event->happen_ts =
                    server_event->happen_ts + shutdown_delay_;

                schedule_callback_(shutdown_finished_event->happen_ts,
                                   shutdown_finished_event, false);

                break;
            }
            case ServerEventType::kShutdownFinished: {
                // switch state to kOff

                if (state_ != ServerState::kTurningOn) {
                    LOG_F(
                        ERROR,
                        "ShutDownFinished event received, but server is not in "
                        "TurningOff state");
                    state_ = ServerState::kFailure;
                    break;
                }

                state_ = ServerState::kOff;

                break;
            }
            case ServerEventType::kProvisionVM: {
                // add VM to list of hosted VMs and schedule event for VM
                // startup

                if (state_ != ServerState::kRunning) {
                    LOG_F(ERROR,
                          "ProvisionVM event received, but server is not in "
                          "Running state");
                    state_ = ServerState::kFailure;
                    break;
                }

                if (!server_event->virtual_machine) {
                    LOG_F(ERROR,
                          "ProvisionVM event without virtual_machine attached");
                    break;
                }

                virtual_machines_.push_back(server_event->virtual_machine);

                // TODO: schedule event

                break;
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
