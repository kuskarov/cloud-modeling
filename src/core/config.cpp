#include "config.h"

#include <yaml-cpp/yaml.h>

#include <argparse/argparse.hpp>

#include "data-center.h"
#include "server.h"

bool
sim::core::SimulatorConfig::ParseArgs(int argc, char** argv)
{
    argparse::ArgumentParser parser("simulator");

    parser.add_argument("--verbose")
        .help("Print additional logs")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--resources-file")
        .help("Path to YAML config file with resources description")
        .nargs(1)
        .required();

    parser.add_argument("--tasks-file")
        .help("Path to YAML config file with tasks description")
        .nargs(1);

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& re) {
        // log error!
        return false;
    }

    if (parser.get<bool>("--verbose")) {
        verbose_ = true;
    }

    resources_config_path_ = parser.get<std::string>("--resources-file");
    // tasks_config_path_ = parser.get<std::string>("--tasks-file");

    return true;
}

bool
sim::core::SimulatorConfig::ParseResources(
    const std::function<void(resources::DataCenter)>& add_data_center)
{
    YAML::Node file = YAML::LoadFile(resources_config_path_);
    if (file["data-centers"]) {
        YAML::Node dc_config_array = file["data-centers"];

        if (!dc_config_array.IsSequence()) {
            return false;
        }

        for (const auto& dc_config : dc_config_array) {
            resources::DataCenter data_center;

            if (!dc_config["name"]) {
                return false;
            }
            data_center.SetName(dc_config["name"].as<std::string>());

            if (!dc_config["servers"] || !dc_config["servers"].IsSequence()) {
                return false;
            }
            for (const auto& server_config : dc_config["servers"]) {
                resources::Server server{};

                if (!server_config["name"]) {
                    return false;
                }
                server.SetName(server_config["name"].as<std::string>());

                if (!server_config["ram"]) {
                    return false;
                }
                server.SetRam(server_config["ram"].as<types::RAMBytes>());

                if (!server_config["clock-rate"]) {
                    return false;
                }
                server.SetClockRate(
                    server_config["clock-rate"].as<types::CPUHertz>());

                if (!server_config["cores-count"]) {
                    return false;
                }
                server.SetCoresCount(
                    server_config["cores-count"].as<uint32_t>());

                if (!server_config["count"]) {
                    return false;
                }
                auto count = server_config["count"].as<uint32_t>();

                data_center.AddServers(server, count);
            }

            add_data_center(data_center);
        }
    }

    return true;
}

bool
sim::core::SimulatorConfig::ParseTasks()
{
    YAML::Node file = YAML::LoadFile(tasks_config_path_);

    return false;
}
