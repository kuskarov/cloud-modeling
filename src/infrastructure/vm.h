#pragma once

#include "actor.h"

namespace sim::infra {

enum class VMEventType
{
    kNone,
    kProvisionCompleted,   // sent by Server when VM is located on a
                           // concrete server
    kStart,                // scheduled in ProvisionCompleted handler
    kStartCompleted,       // scheduled in Start handler
    kRestart,              // external command, VM should be in Running state
    kRestartCompleted,     // scheduled in Restart handler
    kStop,                 // external command, VM should be in Running state
    kStopCompleted,        // scheduled in Stop handler
    kDelete,               // external command, VM should be in Running state
    kDeleteCompleted,      // scheduled in DeleteCompleted handler
};

struct VMEvent : events::Event
{
    VMEventType type{VMEventType::kNone};

    UUID server_uuid;
};

/// Used https://cloud.yandex.ru/docs/compute/concepts/vm-statuses
enum class VMState
{
    kNone,
    kProvisioning,   // default (set immediately after the creation)
    kStarting,       // VM is located on a server, but is not running yet
    kRunning,
    kRestarting,
    kStopping,
    kStopped,   // moved to VM image storage and can be
    kDeleting,
    kFailure,
};

/**
 * This struct is returned by VM on each call of GetWorkload(). Should be
 * implemented by researcher
 */
struct IVMWorkload
{
    virtual ~IVMWorkload() = default;
};

/**
 * Will be called each tick, should match with IWorkload impl
 */
typedef std::function<std::shared_ptr<IVMWorkload>(TimeStamp)> WorkloadFunction;

/**
 * Representation of Virtual Machine.
 *
 * VM is an Actor, but it is not a Resource, as VM's life cycle is different
 * from physical entities
 */
class VM : public events::IActor
{
 public:
    VM() : events::IActor("VM") {}

    void HandleEvent(const events::Event* event) override;

    [[nodiscard]] TimeInterval GetStartDelay() const;
    void SetStartDelay(sim::TimeInterval start_delay);
    [[nodiscard]] TimeInterval GetRestartDelay() const;
    void SetRestartDelay(sim::TimeInterval restart_delay);
    [[nodiscard]] TimeInterval GetStopDelay() const;
    void SetStopDelay(sim::TimeInterval stop_delay);
    [[nodiscard]] TimeInterval GetDeleteDelay() const;
    void SetDeleteDelay(sim::TimeInterval delete_delay);

    // TODO: get access to the time for actors!
    [[nodiscard]] auto GetWorkload() const
    {
        return workload_function_(TimeStamp{0});
    }

    void SetWorkloadFunction(const WorkloadFunction& workload_function)
    {
        workload_function_ = workload_function;
    }

 private:
    VMState state_{VMState::kProvisioning};

    WorkloadFunction workload_function_;

    inline void SetState(VMState new_state);

    TimeInterval start_delay_{0}, restart_delay_{0}, stop_delay_{0},
        delete_delay_{0};

    // event handlers
    void CompleteProvision(const VMEvent* vm_event);
    void Start(const VMEvent* vm_event);
    void CompleteStart(const VMEvent* vm_event);
    void Restart(const VMEvent* vm_event);
    void CompleteRestart(const VMEvent* vm_event);
    void Stop(const VMEvent* vm_event);
    void CompleteStop(const VMEvent* vm_event);
    void Delete(const VMEvent* vm_event);
    void CompleteDelete(const VMEvent* vm_event);
};

}   // namespace sim::infra
