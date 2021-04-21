#include "vm.h"

#include "logger.h"

void
sim::infra::VM::HandleEvent(const events::Event* event)
{
    try {
        auto vm_event = dynamic_cast<const VMEvent*>(event);

        if (!vm_event) {
            ACTOR_LOG_ERROR("Ptr is null, why?");
            state_ = VMState::kFailure;
            return;
        }

        switch (vm_event->type) {
            case VMEventType::kProvisionCompleted: {
                if (!StateIs(VMState::kProvisioning,
                             "Provision Completed Event handler")) {
                    break;
                }

                auto next_event = new VMEvent();
                next_event->happen_time = vm_event->happen_time;
                next_event->type = VMEventType::kStart;
                next_event->addressee = this;

                schedule_event(next_event, true);

                break;
            }
            case VMEventType::kStart: {
                if (!StateIs(VMState::kProvisioning, "Start Event handler")) {
                    break;
                }

                state_ = VMState::kStarting;
                ACTOR_LOG_INFO("State changed to {}", StateToString(state_));

                auto next_event = new VMEvent();
                next_event->happen_time = vm_event->happen_time + start_delay_;
                next_event->type = VMEventType::kStartCompleted;
                next_event->addressee = this;

                schedule_event(next_event, false);

                break;
            }
            case VMEventType::kStartCompleted: {
                if (!StateIs(VMState::kStarting,
                             "Start Completed Event handler")) {
                    break;
                }

                state_ = VMState::kRunning;
                ACTOR_LOG_INFO("State changed to {}", StateToString(state_));

                // TODO: schedule notification to the caller

                break;
            }
            case VMEventType::kRestart: {
                if (!StateIs(VMState::kRunning, "Restart Event handler")) {
                    break;
                }

                state_ = VMState::kRestarting;
                ACTOR_LOG_INFO("State changed to {}", StateToString(state_));

                auto next_event = new VMEvent();
                next_event->happen_time =
                    vm_event->happen_time + restart_delay_;
                next_event->type = VMEventType::kRestartCompleted;
                next_event->addressee = this;

                schedule_event(next_event, false);

                break;
            }
            case VMEventType::kRestartCompleted: {
                if (!StateIs(VMState::kRestarting, "Stop Event handler")) {
                    break;
                }

                state_ = VMState::kRunning;
                ACTOR_LOG_INFO("State changed to {}", StateToString(state_));

                // TODO: schedule notification to the caller

                break;
            }
            case VMEventType::kStop: {
                if (!StateIs(VMState::kRunning, "Stop Event handler")) {
                    break;
                }

                state_ = VMState::kStopping;
                ACTOR_LOG_INFO("State changed to {}", StateToString(state_));

                auto next_event = new VMEvent();
                next_event->happen_time = vm_event->happen_time + stop_delay_;
                next_event->type = VMEventType::kStopCompleted;
                next_event->addressee = this;

                schedule_event(next_event, false);

                break;
            }
            case VMEventType::kStopCompleted: {
                if (!StateIs(VMState::kStopping,
                             "Stop Completed Event handler")) {
                    break;
                }

                state_ = VMState::kStopped;
                ACTOR_LOG_INFO("State changed to {}", StateToString(state_));

                // TODO: schedule notification to the caller

                break;
            }
            case VMEventType::kDelete: {
                if (!StateIs(VMState::kRunning, "Delete Event handler")) {
                    break;
                }

                state_ = VMState::kDeleting;
                ACTOR_LOG_INFO("State changed to {}", StateToString(state_));

                auto next_event = new VMEvent();
                next_event->happen_time = vm_event->happen_time + delete_delay_;
                next_event->type = VMEventType::kDeleteCompleted;
                next_event->addressee = this;

                schedule_event(next_event, false);

                break;
            }
            case VMEventType::kDeleteCompleted: {
                if (!StateIs(VMState::kDeleting,
                             "Delete Completed Event handler")) {
                    break;
                }

                state_ = VMState::kNone;
                ACTOR_LOG_INFO("State changed to {}", StateToString(state_));

                // TODO: schedule notification to the caller

                break;
            }
            default: {
                ACTOR_LOG_ERROR("Received VM event with invalid type",
                                name_.c_str());
                break;
            }
        }

    } catch (const std::bad_cast& bc) {
        ACTOR_LOG_ERROR("Received non-VM event!", name_.c_str());
    }
}

sim::types::TimeInterval
sim::infra::VM::GetStartDelay() const
{
    return start_delay_;
}

void
sim::infra::VM::SetStartDelay(types::TimeInterval start_delay)
{
    start_delay_ = start_delay;
}

sim::types::TimeInterval
sim::infra::VM::GetRestartDelay() const
{
    return restart_delay_;
}

void
sim::infra::VM::SetRestartDelay(types::TimeInterval restart_delay)
{
    restart_delay_ = restart_delay;
}

sim::types::TimeInterval
sim::infra::VM::GetStopDelay() const
{
    return stop_delay_;
}

void
sim::infra::VM::SetStopDelay(types::TimeInterval stop_delay)
{
    stop_delay_ = stop_delay;
}

sim::types::TimeInterval
sim::infra::VM::GetDeleteDelay() const
{
    return delete_delay_;
}

void
sim::infra::VM::SetDeleteDelay(types::TimeInterval delete_delay)
{
    delete_delay_ = delete_delay;
}

bool
sim::infra::VM::StateIs(VMState expected, const std::string& caller_info)
{
    if (state_ != expected) {
        ACTOR_LOG_ERROR("{}: given state {}, expected {}", caller_info.c_str(),
                        StateToString(state_), StateToString(expected));

        state_ = VMState::kFailure;

        return false;
    }
    return true;
}

const char*
sim::infra::StateToString(VMState state)
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
