#pragma once

#include <string>

#include "data-center.h"

namespace sim::core {

class SimulatorConfig
{
 public:
    void ParseArgs(int argc, char** argv);
    void ParseResources(
        const std::function<void(std::shared_ptr<resources::DataCenter>)>&
            add_data_center);

 private:
    std::string resources_config_path_{};
};

}   // namespace sim::core
