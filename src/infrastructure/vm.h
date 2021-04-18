#pragma once

#include "actor.h"

namespace sim::resources {

enum class VirtualMachineEventType
{
    kNone,
    kBoot,             // called when the server is Off
    kBootFinished,     // scheduled by kBoot and kReboot handlers
    kStop,             // called when the server is Running
    kStopFinished,     // scheduled by kStop
    kDelete,           // TODO
    kDeleteFinished,   // TODO
};

struct VirtualMachineEvent : events::Event
{
    VirtualMachineEventType type{VirtualMachineEventType::kNone};
};

class VirtualMachine : public events::Actor
{
 public:
    void HandleEvent(const std::shared_ptr<events::Event>& event) override;

 private:
};

}   // namespace sim::resources
