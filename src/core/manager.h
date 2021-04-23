#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "actor.h"
#include "cloud.h"
#include "config.h"
#include "event-loop.h"
#include "resource-register.h"
#include "scheduler.h"
#include "vm-storage.h"

namespace sim::core {

class Manager : public events::IActor
{
 public:
    explicit Manager(std::shared_ptr<SimulatorConfig> config)
        : events::IActor("Simulator"), config_(std::move(config))
    {
    }

    void HandleEvent(const events::Event* event) override {}

    void Setup();
    void Listen();

 private:
    std::shared_ptr<infra::Cloud> cloud_{};

    std::shared_ptr<infra::VMStorage> vm_storage_{};

    std::shared_ptr<IScheduler> scheduler_{};

    std::shared_ptr<SimulatorConfig> config_;

    std::shared_ptr<events::EventLoop> event_loop_;

    std::shared_ptr<infra::ResourceRegister> register_{};

    // event handlers
    void Boot(const std::string& command);
    void CreateVM(const std::string& command);
    void ProvisionVM(const std::string& command);
    void StopVM(const std::string& command);
    void Shutdown(const std::string& command);
    void DeleteVM(const std::string& command);
};

}   // namespace sim::core
