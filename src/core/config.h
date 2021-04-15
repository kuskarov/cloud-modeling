#pragma once

#include <string>

#include "data-center.h"

namespace sim::core {

class SimulatorConfig
{
 public:
    void ParseArgs(int argc, char **argv);
    void ParseResources(
        const std::function<void(resources::DataCenter)> &add_data_center);
    bool ParseTasks();

 private:
    bool verbose_{};
    std::string resources_config_path_{}, tasks_config_path_{};
};

}   // namespace sim::core
