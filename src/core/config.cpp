#include "config.h"

#include <yaml-cpp/yaml.h>

#include <argparse/argparse.hpp>

#include "data-center.h"
#include "file-utils.h"
#include "logger.h"
#include "server.h"

void
sim::core::SimulatorConfig::ParseArgs(int argc, char** argv)
{
    argparse::ArgumentParser parser("simulator");

    parser.add_argument("--resources-file")
        .help("Path to YAML config file with resources description")
        .nargs(1)
        .required();

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& re) {
        SimulatorLogger().LogRawError("Parse error: {}", re.what());
        throw;
    }

    resources_config_path_ = parser.get<std::string>("--resources-file");
}

#define CHECK(condition, ...)                                       \
    {                                                               \
        if (!condition) SimulatorLogger().LogRawError(__VA_ARGS__); \
    }

void
sim::core::SimulatorConfig::ParseResources(
    const std::function<void(std::shared_ptr<infra::DataCenter>)>&
        add_data_center,
    const events::ScheduleFunction& schedule_function)
{
    CHECK(FileExists(resources_config_path_), "File {} does not exist",
          resources_config_path_.c_str());
    YAML::Node file = YAML::LoadFile(resources_config_path_);

    CHECK(file["data-centers"],
          "No \"data-centers\" field in data-center spec");
    YAML::Node dc_config_array = file["data-centers"];

    CHECK(dc_config_array.IsSequence(),
          "Value of a key \"data-centers\" is not a sequence");
    for (const auto& dc_config : dc_config_array) {
        auto data_center = std::make_shared<infra::DataCenter>();

        data_center->SetScheduleFunction(schedule_function);

        CHECK(dc_config["name"], "No \"name\" field in data-center spec");
        data_center->SetName(dc_config["name"].as<std::string>());

        CHECK(dc_config["servers"], "No \"servers\" field in data-center spec");
        CHECK(dc_config["servers"].IsSequence(),
              "Value of a key \"servers\" is not a sequence");
        for (const auto& server_config : dc_config["servers"]) {
            infra::Server server_template{};

            server_template.SetOwner(data_center.get());
            server_template.SetScheduleFunction(schedule_function);

            CHECK(server_config["name"], "No \"name\" field in server spec");
            server_template.SetName(server_config["name"].as<std::string>());

            CHECK(server_config["ram"], "No \"ram\" field in server spec");
            server_template.SetRam(server_config["ram"].as<types::RAMBytes>());

            CHECK(server_config["clock-rate"],
                  "No \"clock-rate\" field in server spec");

            // TODO: write in a normal way
            server_template.SetClockRate(static_cast<types::CPUHertz>(
                server_config["clock-rate"].as<float>() * 1'000'000));

            CHECK(server_config["cores-count"],
                  "No \"cores-count\" field in server spec");
            server_template.SetCoresCount(
                server_config["cores-count"].as<uint32_t>());

            CHECK(server_config["cost"], "No \"cost\" field in server spec");
            server_template.SetCost(
                server_config["cost"].as<types::Currency>());

            CHECK(server_config["count"], "No \"count\" field in server spec");
            auto count = server_config["count"].as<uint32_t>();

            /// TODO: write class to create a colony of servers from template
            for (uint32_t i = 0; i < count; ++i) {
                auto server = std::make_shared<infra::Server>(server_template);

                server->SetName(server_template.GetName() + "-" +
                                std::to_string(data_center->ServersCount()));

                data_center->AddServer(server);
            }
        }

        add_data_center(data_center);
    }
}
