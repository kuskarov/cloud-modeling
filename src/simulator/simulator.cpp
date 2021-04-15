#include <iostream>
#include <loguru.hpp>
#include <memory>

#include "cloud-manager.h"
#include "config.h"

int
main(int argc, char** argv)
{
    loguru::init(argc, argv);

    // to be called if CHECK_F fails instead of aborting the whole program
    loguru::set_fatal_handler([](const loguru::Message& message) {
        throw std::runtime_error(std::string(message.prefix) + message.message);
    });

    LOG_F(INFO, "Parsing command-line arguments...");
    auto config = std::make_shared<sim::core::SimulatorConfig>();
    try {
        config->ParseArgs(argc, argv);
    } catch (const std::runtime_error& re) {
        LOG_F(ERROR, "Failed to parse command-line arguments: %s", re.what());
        return 1;
    }
    LOG_F(INFO, "Parsing command-line arguments: Done");

    LOG_F(INFO, "Setting up cloud manager...");
    auto cloud_manager = std::make_shared<sim::core::CloudManager>(config);
    try {
        cloud_manager->Setup();
    } catch (const std::runtime_error& re) {
        LOG_F(ERROR, "Cloud Manager setup failed: %s", re.what());
        return 1;
    }
    LOG_F(INFO, "Setting up cloud manager: Done");

    LOG_F(INFO, "Running cloud manager...");
    cloud_manager->RunCloud();
}
