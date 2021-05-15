#include "vm.h"

#include "server.h"

static const char*
StateToString(sim::infra::VMState state)
{
    switch (state) {
        case sim::infra::VMState::kProvisioning:
            return "PROVISIONING";
        case sim::infra::VMState::kStarting:
            return "STARTING";
        case sim::infra::VMState::kRunning:
            return "RUNNING";
        case sim::infra::VMState::kRestarting:
            return "RESTARTING";
        case sim::infra::VMState::kStopping:
            return "STOPPING";
        case sim::infra::VMState::kStopped:
            return "STOPPED";
        case sim::infra::VMState::kDeleting:
            return "DELETING";
        case sim::infra::VMState::kFailure:
            return "FAILURE";
        default:
            abort();
    }
}

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

    SetOwner(vm_event->server_uuid);

    auto next_event =
        events::MakeInheritedEvent<VMEvent>(UUID(), vm_event, start_delay_);
    next_event->type = VMEventType::kStart;

    schedule_event(next_event, true);
}

void
sim::infra::VM::Start(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kProvisioning, "Boot Event handler");

    SetState(VMState::kStarting);

    auto next_event =
        events::MakeInheritedEvent<VMEvent>(UUID(), vm_event, start_delay_);
    next_event->type = VMEventType::kStartCompleted;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteStart(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kStarting, "Boot Completed Event handler");

    SetState(VMState::kRunning);

    auto event = vm_event->notificator;
    event->happen_time = vm_event->happen_time;
    schedule_event(event, false);
}

void
sim::infra::VM::Restart(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kRunning, "Restart Event handler");

    SetState(VMState::kRestarting);

    auto next_event =
        events::MakeInheritedEvent<VMEvent>(UUID(), vm_event, restart_delay_);
    next_event->type = VMEventType::kRestartCompleted;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteRestart(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kRestarting, "Stop Event handler");

    SetState(VMState::kRunning);

    auto event = vm_event->notificator;
    event->happen_time = vm_event->happen_time;
    schedule_event(event, false);
}

void
sim::infra::VM::Stop(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kRunning, "Stop Event handler");

    SetState(VMState::kStopping);

    auto next_event =
        events::MakeInheritedEvent<VMEvent>(UUID(), vm_event, stop_delay_);
    next_event->type = VMEventType::kStopCompleted;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteStop(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kStopping, "Stop Completed Event handler");

    SetState(VMState::kStopped);

    auto free_server_event = events::MakeInheritedEvent<ServerEvent>(
        owner_, vm_event, types::TimeInterval{0});
    free_server_event->type = ServerEventType::kUnprovisionVM;
    free_server_event->vm_uuid = UUID();

    schedule_event(free_server_event, false);

    owner_ = types::UUID{};
}

void
sim::infra::VM::Delete(const sim::infra::VMEvent* vm_event)
{
    SetState(VMState::kDeleting);

    auto next_event =
        events::MakeInheritedEvent<VMEvent>(UUID(), vm_event, delete_delay_);
    next_event->type = VMEventType::kDeleteCompleted;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteDelete(const sim::infra::VMEvent* vm_event)
{
    CHECK_STATE(VMState::kDeleting, "Delete Completed Event handler");

    if (owner_) {
        auto free_server_event = events::MakeEvent<ServerEvent>(
            owner_, vm_event->happen_time, vm_event->notificator);
        free_server_event->type = ServerEventType::kUnprovisionVM;
        free_server_event->vm_uuid = UUID();

        schedule_event(free_server_event, false);
    } else {
        auto event = vm_event->notificator;
        event->happen_time = vm_event->happen_time;
        schedule_event(event, false);
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

void
sim::infra::VM::SetState(sim::infra::VMState new_state)
{
    state_ = new_state;
    ACTOR_LOG_INFO("State changed to {}", StateToString(new_state));
}
