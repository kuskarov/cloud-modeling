#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "actor.h"
#include "cloud.h"
#include "config.h"
#include "event-loop.h"
#include "scheduler.h"
#include "vm-storage.h"

namespace sim::core {

class Manager : public events::Actor
{
 public:
    explicit Manager(std::shared_ptr<SimulatorConfig> config)
        : events::Actor("Simulator"), config_(std::move(config))
    {
    }

    void HandleEvent(const std::shared_ptr<events::Event>& event) override {}

    void Setup();
    void Listen();

 private:
    std::shared_ptr<infra::Cloud> cloud_{};

    std::shared_ptr<VMStorage> vm_storage_{};

    std::shared_ptr<Scheduler> scheduler_{};

    std::shared_ptr<SimulatorConfig> config_;

    std::shared_ptr<events::EventLoop> event_loop_;

    // event handlers
    void CreateVM(const std::string& command);
    void Provision(const std::string& command);
};

}   // namespace sim::core
