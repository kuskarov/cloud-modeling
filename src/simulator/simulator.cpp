#include <iostream>
#include <memory>

#include "config.h"
#include "logger.h"
#include "manager.h"

int
main(int argc, char** argv)
{
    std::shared_ptr<sim::core::Manager> manager{};

    try {
        std::cout << "Parsing command-line arguments..." << std::endl;

        auto config = std::make_shared<sim::core::SimulatorConfig>();

        config->ParseArgs(argc, argv);

        std::cout << "Parsing command-line arguments: Done" << std::endl;

        std::cout << "Setting up cloud manager..." << std::endl;

        manager = std::make_shared<sim::core::Manager>(config);

        manager->Setup();

        std::cout << "Setting up cloud manager: Done" << std::endl;

        std::cout << "Running cloud manager..." << std::endl;

    } catch (const std::runtime_error& re) {
        std::cout << "Error: " << re.what() << std::endl;
        return 1;
    }

    manager->Listen();
}
