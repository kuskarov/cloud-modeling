#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "actor-register.h"
#include "actor.h"
#include "cloud.h"
#include "config.h"
#include "event-loop.h"
#include "resource-scheduler.h"
#include "scheduler.h"
#include "vm-storage.h"

namespace sim::core {

class Manager
{
 public:
    explicit Manager(std::shared_ptr<SimulatorConfig> config)
        : whoami_("Manager"), config_(std::move(config))
    {
    }

    void Setup();
    void Listen();

 private:
    std::string whoami_{};

    std::unique_ptr<events::EventLoop> event_loop_;

    events::ScheduleFunction schedule_event;

    std::unique_ptr<ActorRegister> actor_register_;

    std::shared_ptr<SimulatorConfig> config_;

    std::unique_ptr<ServerSchedulerManager> server_scheduler_manager_;

    types::UUID scheduler_handle_{types::NoneUUID()};

    types::UUID cloud_handle_{types::NoneUUID()};

    types::UUID vm_storage_handle_{types::NoneUUID()};

    void Boot(const std::string& command);
    void CreateVM(const std::string& command);
    void ProvisionVM(const std::string& command);
    void StopVM(const std::string& command);
    void Shutdown(const std::string& command);
    void DeleteVM(const std::string& command);

    types::UUID ResolveNameFromInput(const std::string& command);
};

}   // namespace sim::core
