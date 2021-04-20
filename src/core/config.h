#pragma once

#include <string>

#include "data-center.h"

namespace sim::core {

class SimulatorConfig
{
 public:
    void ParseArgs(int argc, char** argv);
    void ParseResources(
        const std::function<void(std::shared_ptr<infra::DataCenter>)>&
            add_data_center,
        const std::function<void(types::TimeStamp,
                                 const std::shared_ptr<events::Event>&, bool)>&
            schedule_callback);

 private:
    std::string resources_config_path_{};
};

}   // namespace sim::core
