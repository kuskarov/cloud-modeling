#include "resource.h"

#include <loguru.hpp>

void
sim::resources::Resource::HandleEvent(
    const std::shared_ptr<events::Event>& event)
{
    try {
        auto resource_event = dynamic_cast<const ResourceEvent*>(event.get());

        switch (resource_event->resource_event_type) {
            case ResourceEventType::kBoot: {
                // switch state to kTurningOn and schedule kBootFinished event

                if (!PowerStateIs(ResourcePowerState::kOff,
                                  "Boot Event Handler")) {
                    break;
                }

                power_state_ = ResourcePowerState::kTurningOn;
                LOG_F(INFO, "State changed to %s",
                      PowerStateToString(power_state_).c_str());

                auto boot_finished_event = std::make_shared<ResourceEvent>();
                boot_finished_event->addressee = this;
                boot_finished_event->resource_event_type =
                    ResourceEventType::kBootFinished;
                boot_finished_event->happen_ts =
                    resource_event->happen_ts + startup_delay_;

                schedule_callback_(boot_finished_event->happen_ts,
                                   boot_finished_event, false);

                break;
            }
            case ResourceEventType::kReboot: {
                break;
            }
            case ResourceEventType::kBootFinished: {
                // switch state to kRunning

                if (!PowerStateIs(ResourcePowerState::kTurningOn,
                                  "BootFinished Event Handler")) {
                    break;
                }

                power_state_ = ResourcePowerState::kRunning;
                LOG_F(INFO, "State changed to %s",
                      PowerStateToString(power_state_).c_str());

                break;
            }
            case ResourceEventType::kShutdown: {
                // switch state to kTurningOff and schedule kShutdownFinished
                // event

                if (!PowerStateIs(ResourcePowerState::kRunning,
                                  "Shutdown Event Handler")) {
                    break;
                }

                power_state_ = ResourcePowerState::kTurningOff;
                LOG_F(INFO, "State changed to %s",
                      PowerStateToString(power_state_).c_str());

                auto shutdown_finished_event =
                    std::make_shared<ResourceEvent>();
                shutdown_finished_event->addressee = this;
                shutdown_finished_event->resource_event_type =
                    ResourceEventType::kShutdownFinished;
                shutdown_finished_event->happen_ts =
                    resource_event->happen_ts + shutdown_delay_;

                schedule_callback_(shutdown_finished_event->happen_ts,
                                   shutdown_finished_event, false);

                break;
            }
            case ResourceEventType::kShutdownFinished: {
                // switch state to kOff

                if (!PowerStateIs(ResourcePowerState::kTurningOff,
                                  "ShutdownFinished Event Handler")) {
                    break;
                }

                power_state_ = ResourcePowerState::kOff;
                LOG_F(INFO, "State changed to %s",
                      PowerStateToString(power_state_).c_str());

                break;
            }
            default: {
                LOG_F(ERROR,
                      "resource %s received resource event with invalid type",
                      name_.c_str());
                break;
            }
        }

    } catch (const std::bad_cast& bc) {
        LOG_F(ERROR, "Resource %s unknown event!", name_.c_str());
    }
}

bool
sim::resources::Resource::PowerStateIs(
    sim::resources::ResourcePowerState expected, const std::string& caller_info)
{
    if (power_state_ != expected) {
        LOG_F(ERROR, "%s: given state %s, expected %s", caller_info.c_str(),
              PowerStateToString(power_state_).c_str(),
              PowerStateToString(expected).c_str());

        power_state_ = ResourcePowerState::kFailure;

        return false;
    }
    return true;
}
