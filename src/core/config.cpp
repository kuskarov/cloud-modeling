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
}

void
sim::core::SimulatorConfig::ParseResources(
    const std::function<void(std::shared_ptr<resources::DataCenter>)>&
        add_data_center)
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
        auto data_center = std::make_shared<resources::DataCenter>();

        CHECK_F(bool(dc_config["name"]),
                "No \"name\" field in data-center spec");
        data_center->SetName(dc_config["name"].as<std::string>());

        CHECK_F(bool(dc_config["servers"]),
                "No \"servers\" field in data-center spec");
        CHECK_F(dc_config["servers"].IsSequence(),
                "Value of a key \"servers\" is not a sequence");
        for (const auto& server_config : dc_config["servers"]) {
            resources::Server server_template{};

            CHECK_F(bool(server_config["name"]),
                    "No \"name\" field in server spec");
            server_template.SetName(server_config["name"].as<std::string>());

            CHECK_F(bool(server_config["ram"]),
                    "No \"ram\" field in server spec");
            server_template.SetRam(server_config["ram"].as<types::RAMBytes>());

            CHECK_F(bool(server_config["clock-rate"]),
                    "No \"clock-rate\" field in server spec");

            // TODO: write in a normal way
            server_template.SetClockRate(static_cast<types::CPUHertz>(
                server_config["clock-rate"].as<float>() * 1'000'000));

            CHECK_F(bool(server_config["cores-count"]),
                    "No \"cores-count\" field in server spec");
            server_template.SetCoresCount(
                server_config["cores-count"].as<uint32_t>());

            CHECK_F(bool(server_config["count"]),
                    "No \"count\" field in server spec");
            auto count = server_config["count"].as<uint32_t>();

            /// TODO: write class to create a colony of servers from template
            for (uint32_t i = 0; i < count; ++i) {
                auto server =
                    std::make_shared<resources::Server>(server_template);

                server->SetName(server_template.GetName() + "-" +
                                std::to_string(data_center->ServersCount()));

                data_center->AddServer(server);
            }
        }

        add_data_center(data_center);
    }
}
