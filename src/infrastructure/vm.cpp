#include "vm.h"

#define CHECK_STATE(expected, extra_text)                                      \
    if (state_ != (expected)) {                                                \
        ACTOR_LOG_ERROR("{}: current state is {} but expected {}", extra_text, \
                        StateToString(state_), StateToString(expected));       \
        SetState(VMState::kFailure);                                           \
        return;                                                                \
    }

void
sim::infra::VM::HandleEvent(const events::Event* event)
{
    try {
        auto vm_event = dynamic_cast<const VMEvent*>(event);

        if (!vm_event) {
            ACTOR_LOG_ERROR("Ptr is null, why?");
            SetState(VMState::kFailure);
            return;
        }

        switch (vm_event->type) {
            case VMEventType::kProvisionCompleted: {
                CompleteProvision(vm_event);
                break;
            }
            case VMEventType::kStart: {
                Start(vm_event);
                break;
            }
            case VMEventType::kStartCompleted: {
                CompleteStart(vm_event);
                break;
            }
            case VMEventType::kRestart: {
                Restart(vm_event);
                break;
            }
            case VMEventType::kRestartCompleted: {
                CompleteRestart(vm_event);
                break;
            }
            case VMEventType::kStop: {
                Stop(vm_event);
                break;
            }
            case VMEventType::kStopCompleted: {
                CompleteStop(vm_event);
                break;
            }
            case VMEventType::kDelete: {
                Delete(vm_event);
                break;
            }
            case VMEventType::kDeleteCompleted: {
                CompleteDelete(vm_event);
                break;
            }
            default: {
                ACTOR_LOG_ERROR("Received VM event with invalid type", name_);
                break;
            }
        }

    } catch (const std::bad_cast& bc) {
        ACTOR_LOG_ERROR("Received non-VM event!", name_);
    }
}

void
sim::infra::VM::CompleteProvision(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kProvisioning, "Provision Completed Event handler");

    auto next_event = new VMEvent();
    next_event->happen_time = vm_event->happen_time;
    next_event->type = VMEventType::kStart;
    next_event->addressee = this;

    schedule_event(next_event, true);
}

void
sim::infra::VM::Start(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kProvisioning, "Start Event handler");

    SetState(VMState::kStarting);

    auto next_event = new VMEvent();
    next_event->happen_time = vm_event->happen_time + start_delay_;
    next_event->type = VMEventType::kStartCompleted;
    next_event->addressee = this;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteStart(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kStarting, "Start Completed Event handler");

    SetState(VMState::kRunning);

    // TODO: schedule notification to the caller
}

void
sim::infra::VM::Restart(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kRunning, "Restart Event handler");

    SetState(VMState::kRestarting);

    auto next_event = new VMEvent();
    next_event->happen_time = vm_event->happen_time + restart_delay_;
    next_event->type = VMEventType::kRestartCompleted;
    next_event->addressee = this;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteRestart(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kRestarting, "Stop Event handler");

    SetState(VMState::kRunning);

    // TODO: schedule notification to the caller
}

void
sim::infra::VM::Stop(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kRunning, "Stop Event handler");

    SetState(VMState::kStopping);

    auto next_event = new VMEvent();
    next_event->happen_time = vm_event->happen_time + stop_delay_;
    next_event->type = VMEventType::kStopCompleted;
    next_event->addressee = this;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteStop(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kStopping, "Stop Completed Event handler");

    SetState(VMState::kStopped);

    // TODO: schedule notification to the caller
}

void
sim::infra::VM::Delete(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kRunning, "Delete Event handler");

    SetState(VMState::kDeleting);

    auto next_event = new VMEvent();
    next_event->happen_time = vm_event->happen_time + delete_delay_;
    next_event->type = VMEventType::kDeleteCompleted;
    next_event->addressee = this;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteDelete(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kDeleting, "Delete Completed Event handler");

    SetState(VMState::kNone);

    // TODO: schedule notification to the caller
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

void
sim::infra::VM::SetState(sim::infra::VMState new_state)
{
    state_ = new_state;
    ACTOR_LOG_INFO("State changed to {}", StateToString(new_state));
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
