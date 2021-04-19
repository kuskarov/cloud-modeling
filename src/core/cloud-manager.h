#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "actor.h"
#include "config.h"
#include "data-center.h"
#include "event-loop.h"

namespace sim::core {

class CloudManager : public events::Actor
{
 public:
    explicit CloudManager(std::shared_ptr<SimulatorConfig> config)
        : events::Actor("Cloud-Manager"), config_(std::move(config))
    {
    }

    void Setup();
    void RunCloud();

 private:
    void ProvisionVM();

    std::vector<std::shared_ptr<resources::DataCenter>> data_centers_{};

    std::shared_ptr<SimulatorConfig> config_;

    std::shared_ptr<events::EventLoop> event_loop_;
};

}   // namespace sim::core
