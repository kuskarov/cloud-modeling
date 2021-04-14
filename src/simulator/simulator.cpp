#include <iostream>
#include <memory>

#include "cloud-manager.h"
#include "config.h"

int
main(int argc, char** argv)
{
    auto config = std::make_shared<sim::core::SimulatorConfig>();
    if (!config->ParseArgs(argc, argv)) {
        std::cerr << "Failed to parse command-line arguments." << std::endl;
        return 1;
    }

    auto cloud_manager = std::make_shared<sim::core::CloudManager>(config);
    if (!cloud_manager->Setup()) {
        std::cerr << "Cloud Manager setup failed." << std::endl;
        return 1;
    }
}
