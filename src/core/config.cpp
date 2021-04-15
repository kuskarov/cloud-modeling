#include "config.h"

#include <yaml-cpp/yaml.h>

#include <argparse/argparse.hpp>
#include <loguru.hpp>

#include "data-center.h"
#include "file-utils.h"
#include "server.h"

void
sim::core::SimulatorConfig::ParseArgs(int argc, char** argv)
{
    argparse::ArgumentParser parser("simulator");

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
        LOG_F(ERROR, "Parse error: %s\n", re.what());
        throw;
    }

    resources_config_path_ = parser.get<std::string>("--resources-file");
    // tasks_config_path_ = parser.get<std::string>("--tasks-file");
}

void
sim::core::SimulatorConfig::ParseResources(
    const std::function<void(resources::DataCenter)>& add_data_center)
{
    CHECK_F(file_exists(resources_config_path_), "File %s does not exist",
            resources_config_path_.c_str());
    YAML::Node file = YAML::LoadFile(resources_config_path_);

    CHECK_F(bool(file["data-centers"]),
            "No \"data-centers\" field in data-center spec");
    YAML::Node dc_config_array = file["data-centers"];

    CHECK_F(dc_config_array.IsSequence(),
            "Value of a key \"data-centers\" is not a sequence");
    for (const auto& dc_config : dc_config_array) {
        resources::DataCenter data_center;

        CHECK_F(bool(dc_config["name"]),
                "No \"name\" field in data-center spec");
        data_center.SetName(dc_config["name"].as<std::string>());

        CHECK_F(bool(dc_config["servers"]),
                "No \"servers\" field in data-center spec");
        CHECK_F(dc_config["servers"].IsSequence(),
                "Value of a key \"servers\" is not a sequence");
        for (const auto& server_config : dc_config["servers"]) {
            resources::Server server{};

            CHECK_F(bool(server_config["name"]),
                    "No \"name\" field in server spec");
            server.SetName(server_config["name"].as<std::string>());

            CHECK_F(bool(server_config["ram"]),
                    "No \"ram\" field in server spec");
            server.SetRam(server_config["ram"].as<types::RAMBytes>());

            CHECK_F(bool(server_config["clock-rate"]),
                    "No \"clock-rate\" field in server spec");

            // TODO: write in a normal way
            server.SetClockRate(static_cast<types::CPUHertz>(
                server_config["clock-rate"].as<float>() * 1'000'000));

            CHECK_F(bool(server_config["cores-count"]),
                    "No \"cores-count\" field in server spec");
            server.SetCoresCount(server_config["cores-count"].as<uint32_t>());

            CHECK_F(bool(server_config["count"]),
                    "No \"count\" field in server spec");
            auto count = server_config["count"].as<uint32_t>();

            data_center.AddServers(server, count);
        }

        add_data_center(data_center);
    }
}

bool
sim::core::SimulatorConfig::ParseTasks()
{
    YAML::Node file = YAML::LoadFile(tasks_config_path_);

    return false;
}
