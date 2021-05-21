#include <iostream>
#include <memory>

#include "config.h"
#include "logger.h"
#include "world.h"

int
main(int argc, char** argv)
{
    std::shared_ptr<sim::core::World> world{};

    try {
        std::cerr << "Parsing command-line arguments..." << std::endl;

        auto config = std::make_shared<sim::core::SimulatorConfig>();

        config->ParseArgs(argc, argv);

        std::cerr << "Parsing command-line arguments: Done" << std::endl;

        std::cerr << "Setting up world..." << std::endl;

        world = std::make_shared<sim::core::World>(config);

        world->Setup();

        std::cerr << "Setting up world: Done" << std::endl;

        std::cerr << "Running world..." << std::endl;

    } catch (const std::runtime_error& re) {
        std::cerr << "Error: " << re.what() << std::endl;
        return 1;
    }

    world->Listen();
}
