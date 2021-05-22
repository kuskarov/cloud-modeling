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

class SimulatorRPCService;

using namespace sim::infra;
using namespace sim::events;

class World
{
 public:
    explicit World(std::shared_ptr<SimulatorConfig> config)
        : whoami_("World"), config_(std::move(config))
    {
    }

    void Setup();
    void Listen();

    std::string_view WhoAmI() const { return whoami_; }

    void DoResourceAction(const std::string& resource_name,
                          infra::ResourceEventType event_type);

    void CreateVM(const std::string& vm_name,
                  const std::string& vm_workload_model,
                  const std::unordered_map<std::string, std::string>& params);

    void DoProvisionVM(const std::string& vm_name);
    void DoRebootVM(const std::string& vm_name);
    void DoStopVM(const std::string& vm_name);
    void DoDeleteVM(const std::string& vm_name);

    // event-loop commands
    void SimulateAll();

 private:
    std::string whoami_{};

    std::unique_ptr<SimulatorRPCService> server_;

    std::unique_ptr<events::EventLoop> event_loop_;

    std::unique_ptr<events::ActorRegister> actor_register_;

    std::shared_ptr<SimulatorConfig> config_;

    std::unique_ptr<ServerSchedulerManager> server_scheduler_manager_;

    std::unique_ptr<IScheduler> scheduler_;

    UUID cloud_handle_{}, vm_storage_handle_{};

    ScheduleFunction schedule_event;

    UUID ResolveName(const std::string& name);
};

}   // namespace sim::core
