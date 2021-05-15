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
#include "rpc-service.h"
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
    std::unique_ptr<SimulatorRPCService> server_;

    std::string whoami_{};

    std::unique_ptr<events::EventLoop> event_loop_;

    events::ScheduleFunction schedule_event;

    std::unique_ptr<events::ActorRegister> actor_register_;

    std::shared_ptr<SimulatorConfig> config_;

    std::unique_ptr<ServerSchedulerManager> server_scheduler_manager_;

    UUID scheduler_handle_{};

    UUID cloud_handle_{};

    UUID vm_storage_handle_{};
};

}   // namespace sim::core
