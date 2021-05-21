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

#define FAIL_ON_STATE_MISMATCH(...)            \
    {                                          \
        if (!CheckStateMatch(__VA_ARGS__)) {   \
            ACTOR_LOG_ERROR("State mismatch"); \
            SetState(VMState::kFailure);       \
            return;                            \
        }                                      \
    }

void
sim::infra::VM::HandleEvent(const Event* event)
{
    auto vm_event = dynamic_cast<const VMEvent*>(event);

    if (!vm_event) {
        ACTOR_LOG_ERROR("Received invalid event");
        SetState(VMState::kFailure);
        return;
    }

    switch (vm_event->type) {
        case VMEventType::kStart:
            Start(vm_event);
            break;
        case VMEventType::kStartCompleted:
            CompleteStart(vm_event);
            break;
        case VMEventType::kRestart:
            Restart(vm_event);
            break;
        case VMEventType::kRestartCompleted:
            CompleteRestart(vm_event);
            break;
        case VMEventType::kStop:
            Stop(vm_event);
            break;
        case VMEventType::kStopCompleted:
            CompleteStop(vm_event);
            break;
        case VMEventType::kDelete:
            Delete(vm_event);
            break;
        case VMEventType::kDeleteCompleted:
            CompleteDelete(vm_event);
            break;
        default: {
            ACTOR_LOG_ERROR("Received event with invalid type");
            break;
        }
    }
}

void
sim::infra::VM::Start(const VMEvent* vm_event)
{
    FAIL_ON_STATE_MISMATCH({VMState::kProvisioning})

    SetState(VMState::kStarting);

    auto next_event =
        MakeInheritedEvent<VMEvent>(GetUUID(), vm_event, start_delay_);
    next_event->type = VMEventType::kStartCompleted;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteStart(const VMEvent* vm_event)
{
    FAIL_ON_STATE_MISMATCH({VMState::kStarting})

    SetState(VMState::kRunning);

    auto vmst_callback = events::MakeEvent<VMStorageEvent>(
        vm_storage_handle_, vm_event->happen_time, nullptr);
    vmst_callback->vm_uuid = GetUUID();
    vmst_callback->type = VMStorageEventType::kVMHosted;

    schedule_event(vmst_callback, false);
}

void
sim::infra::VM::Restart(const VMEvent* vm_event)
{
    FAIL_ON_STATE_MISMATCH({VMState::kRunning, VMState::kFailure})

    SetState(VMState::kRestarting);

    auto next_event =
        MakeInheritedEvent<VMEvent>(GetUUID(), vm_event, restart_delay_);
    next_event->type = VMEventType::kRestartCompleted;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteRestart(const VMEvent* vm_event)
{
    FAIL_ON_STATE_MISMATCH({VMState::kRestarting})

    SetState(VMState::kRunning);
}

void
sim::infra::VM::Stop(const VMEvent* vm_event)
{
    FAIL_ON_STATE_MISMATCH({VMState::kRunning})

    SetState(VMState::kStopping);

    auto next_event =
        MakeInheritedEvent<VMEvent>(GetUUID(), vm_event, stop_delay_);
    next_event->type = VMEventType::kStopCompleted;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteStop(const VMEvent* vm_event)
{
    FAIL_ON_STATE_MISMATCH({VMState::kStopping})

    SetState(VMState::kStopped);

    auto free_server_event =
        MakeInheritedEvent<ServerEvent>(owner_, vm_event, TimeInterval{0});
    free_server_event->type = ServerEventType::kUnprovisionVM;
    free_server_event->vm_uuid = GetUUID();

    schedule_event(free_server_event, false);

    owner_ = UUID{};
}

void
sim::infra::VM::Delete(const VMEvent* vm_event)
{
    FAIL_ON_STATE_MISMATCH(
        {VMState::kRunning, VMState::kStopped, VMState::kFailure})

    SetState(VMState::kDeleting);

    auto next_event =
        MakeInheritedEvent<VMEvent>(GetUUID(), vm_event, delete_delay_);
    next_event->type = VMEventType::kDeleteCompleted;

    schedule_event(next_event, false);
}

void
sim::infra::VM::CompleteDelete(const VMEvent* vm_event)
{
    FAIL_ON_STATE_MISMATCH({VMState::kDeleting})

    if (owner_) {
        auto free_server_event = MakeEvent<ServerEvent>(
            owner_, vm_event->happen_time, vm_event->notificator);
        free_server_event->type = ServerEventType::kUnprovisionVM;
        free_server_event->vm_uuid = GetUUID();

        schedule_event(free_server_event, false);
    } else {
        auto vmst_callback = events::MakeEvent<VMStorageEvent>(
            vm_storage_handle_, vm_event->happen_time, nullptr);
        vmst_callback->vm_uuid = GetUUID();
        vmst_callback->type = VMStorageEventType::kVMDeleted;

        schedule_event(vmst_callback, false);
    }
}

sim::TimeInterval
sim::infra::VM::GetStartDelay() const
{
    return start_delay_;
}

void
sim::infra::VM::SetStartDelay(TimeInterval start_delay)
{
    start_delay_ = start_delay;
}

sim::TimeInterval
sim::infra::VM::GetRestartDelay() const
{
    return restart_delay_;
}

void
sim::infra::VM::SetRestartDelay(TimeInterval restart_delay)
{
    restart_delay_ = restart_delay;
}

sim::TimeInterval
sim::infra::VM::GetStopDelay() const
{
    return stop_delay_;
}

void
sim::infra::VM::SetStopDelay(TimeInterval stop_delay)
{
    stop_delay_ = stop_delay;
}

sim::TimeInterval
sim::infra::VM::GetDeleteDelay() const
{
    return delete_delay_;
}

void
sim::infra::VM::SetDeleteDelay(TimeInterval delete_delay)
{
    delete_delay_ = delete_delay;
}

void
sim::infra::VM::SetState(sim::infra::VMState new_state)
{
    state_ = new_state;
    ACTOR_LOG_INFO("State changed to {}", StateToString(new_state));
}

bool
sim::infra::VM::CheckStateMatch(std::initializer_list<VMState> allowed_states)
{
    return std::any_of(allowed_states.begin(), allowed_states.end(),
                       [this](VMState state) { return state == state_; });
}
