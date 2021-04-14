#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "actor.h"
#include "config.h"
#include "data-center.h"
#include "event-loop.h"

namespace sim::core {

class CloudManager : public sim::events::Actor
{
 public:
    explicit CloudManager(std::shared_ptr<SimulatorConfig> config)
        : config_(std::move(config))
    {
    }

    bool Setup();
    void RunCloud();

 private:
    std::vector<resources::DataCenter> data_centers_{};
    std::shared_ptr<SimulatorConfig> config_;
    std::shared_ptr<events::EventLoop> event_loop_;
};

}   // namespace sim::core
