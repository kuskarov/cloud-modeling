#include "vm.h"

#include <loguru.hpp>

void
sim::infra::VM::HandleEvent(const std::shared_ptr<events::Event>& event)
{
    try {
        auto vm_event = dynamic_cast<const VMEvent*>(event.get());

        if (!vm_event) {
            LOG_F(ERROR, "Ptr is null, why?");
            state_ = VMState::kFailure;
            return;
        }

        switch (vm_event->type) {
            case VMEventType::kProvisionCompleted: {
                if (!StateIs(VMState::kProvisioning,
                             "Provision Completed Event handler")) {
                    break;
                }

                auto next_event = std::make_shared<VMEvent>();
                next_event->happen_ts = vm_event->happen_ts;
                next_event->type = VMEventType::kStart;
                next_event->addressee = this;

                // do immediately
                schedule_callback_(next_event->happen_ts, next_event, true);

                break;
            }
            case VMEventType::kStart: {
                if (!StateIs(VMState::kProvisioning, "Start Event handler")) {
                    break;
                }

                state_ = VMState::kStarting;
                LOG_F(INFO, "State changed to %s", StateToString(state_));

                auto next_event = std::make_shared<VMEvent>();
                next_event->happen_ts = vm_event->happen_ts + start_delay_;
                next_event->type = VMEventType::kStartCompleted;
                next_event->addressee = this;

                schedule_callback_(next_event->happen_ts, next_event, false);

                break;
            }
            case VMEventType::kStartCompleted: {
                if (!StateIs(VMState::kStarting,
                             "Start Completed Event handler")) {
                    break;
                }

                state_ = VMState::kRunning;
                LOG_F(INFO, "State changed to %s", StateToString(state_));

                // TODO: schedule notification to the caller

                break;
            }
            case VMEventType::kRestart: {
                if (!StateIs(VMState::kRunning, "Restart Event handler")) {
                    break;
                }

                state_ = VMState::kRestarting;
                LOG_F(INFO, "State changed to %s", StateToString(state_));

                auto next_event = std::make_shared<VMEvent>();
                next_event->happen_ts = vm_event->happen_ts + restart_delay_;
                next_event->type = VMEventType::kRestartCompleted;
                next_event->addressee = this;

                schedule_callback_(next_event->happen_ts, next_event, false);

                break;
            }
            case VMEventType::kRestartCompleted: {
                if (!StateIs(VMState::kRestarting, "Stop Event handler")) {
                    break;
                }

                state_ = VMState::kRunning;
                LOG_F(INFO, "State changed to %s", StateToString(state_));

                // TODO: schedule notification to the caller

                break;
            }
            case VMEventType::kStop: {
                if (!StateIs(VMState::kRunning, "Stop Event handler")) {
                    break;
                }

                state_ = VMState::kStopping;
                LOG_F(INFO, "State changed to %s", StateToString(state_));

                auto next_event = std::make_shared<VMEvent>();
                next_event->happen_ts = vm_event->happen_ts + stop_delay_;
                next_event->type = VMEventType::kStopCompleted;
                next_event->addressee = this;

                schedule_callback_(next_event->happen_ts, next_event, false);

                break;
            }
            case VMEventType::kStopCompleted: {
                if (!StateIs(VMState::kStopping,
                             "Stop Completed Event handler")) {
                    break;
                }

                state_ = VMState::kStopped;
                LOG_F(INFO, "State changed to %s", StateToString(state_));

                // TODO: schedule notification to the caller

                break;
            }
            case VMEventType::kDelete: {
                if (!StateIs(VMState::kRunning, "Delete Event handler")) {
                    break;
                }

                state_ = VMState::kDeleting;
                LOG_F(INFO, "State changed to %s", StateToString(state_));

                auto next_event = std::make_shared<VMEvent>();
                next_event->happen_ts = vm_event->happen_ts + delete_delay_;
                next_event->type = VMEventType::kDeleteCompleted;
                next_event->addressee = this;

                schedule_callback_(next_event->happen_ts, next_event, false);

                break;
            }
            case VMEventType::kDeleteCompleted: {
                if (!StateIs(VMState::kDeleting,
                             "Delete Completed Event handler")) {
                    break;
                }

                state_ = VMState::kNone;
                LOG_F(INFO, "State changed to %s", StateToString(state_));

                // TODO: schedule notification to the caller

                break;
            }
            default: {
                LOG_F(ERROR, "VM %s received VM event with invalid type",
                      name_.c_str());
                break;
            }
        }

    } catch (const std::bad_cast& bc) {
        LOG_F(ERROR, "VM %s received non-VM event!", name_.c_str());
    }
}

sim::types::TimeInterval
sim::infra::VM::GetStartDelay() const
{
    return start_delay_;
}

void
sim::infra::VM::SetStartDelay(sim::types::TimeInterval start_delay)
{
    start_delay_ = start_delay;
}

sim::types::TimeInterval
sim::infra::VM::GetRestartDelay() const
{
    return restart_delay_;
}

void
sim::infra::VM::SetRestartDelay(sim::types::TimeInterval restart_delay)
{
    restart_delay_ = restart_delay;
}

sim::types::TimeInterval
sim::infra::VM::GetStopDelay() const
{
    return stop_delay_;
}

void
sim::infra::VM::SetStopDelay(sim::types::TimeInterval stop_delay)
{
    stop_delay_ = stop_delay;
}

sim::types::TimeInterval
sim::infra::VM::GetDeleteDelay() const
{
    return delete_delay_;
}

void
sim::infra::VM::SetDeleteDelay(sim::types::TimeInterval delete_delay)
{
    delete_delay_ = delete_delay;
}

bool
sim::infra::VM::StateIs(sim::infra::VMState expected,
                        const std::string& caller_info)
{
    if (state_ != expected) {
        LOG_F(ERROR, "%s: given state %s, expected %s", caller_info.c_str(),
              StateToString(state_), StateToString(expected));

        state_ = VMState::kFailure;

        return false;
    }
    return true;
}

const char*
sim::infra::StateToString(sim::infra::VMState state)
{
    switch (state) {
        case VMState::kProvisioning:
            return "PROVISIONING";
        case VMState::kStarting:
            return "STARTING";
        case VMState::kRunning:
            return "RUNNING";
        case VMState::kRestarting:
            return "RESTARTING";
        case VMState::kStopping:
            return "STOPPING";
        case VMState::kStopped:
            return "STOPPED";
        case VMState::kDeleting:
            return "DELETING";
        case VMState::kFailure:
            return "FAILURE";
        default:
            return "UNREACHABLE";
    }
}
