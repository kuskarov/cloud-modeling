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

static inline const char* StateToString(VMState state);

/**
 * Representation of Virtual Machine.
 *
 * VM is an Actor, but it is not a Resource, as VM's life cycle is different
 * from physical entities
 */
class VM : public events::Actor
{
 public:
    VM() : events::Actor("VM") {}

    void HandleEvent(const std::shared_ptr<events::Event>& event) override;

    [[nodiscard]] types::TimeInterval GetStartDelay() const;
    void SetStartDelay(sim::types::TimeInterval start_delay);
    [[nodiscard]] types::TimeInterval GetRestartDelay() const;
    void SetRestartDelay(sim::types::TimeInterval restart_delay);
    [[nodiscard]] types::TimeInterval GetStopDelay() const;
    void SetStopDelay(sim::types::TimeInterval stop_delay);
    [[nodiscard]] types::TimeInterval GetDeleteDelay() const;
    void SetDeleteDelay(sim::types::TimeInterval delete_delay);

 private:
    inline bool StateIs(VMState expected, const std::string& caller_info);

    VMState state_{VMState::kProvisioning};

    /// TODO: remove magic numbers
    types::TimeInterval start_delay_{4}, restart_delay_{3}, stop_delay_{8},
        delete_delay_{6};
};

}   // namespace sim::infra
