#include "config.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <argparse.hpp>

#include "cloud.h"
#include "custom-code.h"
#include "data-center.h"
#include "file-utils.h"
#include "logger.h"
#include "server.h"

void
sim::core::SimulatorConfig::ParseArgs(int argc, char** argv)
{
    argparse::ArgumentParser parser("simulator");

    parser.add_argument("--config")
        .help("Path to directory with configuration files")
        .nargs(1)
        .required();

    parser.add_argument("--logs-folder")
        .help("Path to the folder where to write logs")
        .nargs(1)
        .required();

    parser.add_argument("--port")
        .help("Port on which RPC calls will be awaited")
        .nargs(1)
        .required();

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& re) {
        std::cerr << "Argument parse error: " << re.what() << "\n";
        std::cerr << parser << "\n";
        throw;
    }

    config_path_ = parser.get<std::string>("--config");
    logs_path_ = parser.get<std::string>("--logs-folder");
    port_ = std::stoi(parser.get<std::string>("--port"));
}

#define CHECK(condition, ...)                        \
    {                                                \
        if (!(condition)) {                          \
            WORLD_LOG_ERROR(__VA_ARGS__);            \
            throw std::runtime_error("Parse error"); \
        }                                            \
    }

void
sim::core::SimulatorConfig::ParseResources(
    sim::types::UUID cloud_handle, ActorRegister* actor_register,
    ServerSchedulerManager* server_scheduler_manager)
{
    ParseSpecs(config_path_ + "/specs.yaml");
    ParseCloud(config_path_ + "/cloud.yaml", cloud_handle, actor_register,
               server_scheduler_manager);
}

void
sim::core::SimulatorConfig::ParseSpecs(const std::string& specs_file_name)
{
    CHECK(FileExists(specs_file_name), "File {} does not exist",
          specs_file_name);

    YAML::Node specs = YAML::LoadFile(specs_file_name);

    CHECK(specs.IsSequence(), "File {} is not a sequence", specs_file_name);

    for (const auto& spec : specs) {
        auto spec_name = spec["name"];
        auto spec_ram = spec["ram"];
        auto spec_clock_rate = spec["clock-rate"];
        auto spec_cores_count = spec["cores-count"];

        CHECK(spec_name, "Field \"name\" not found");
        CHECK(spec_name.IsScalar(), "Field \"name\" is not a single value");

        CHECK(spec_ram, "Field \"ram\" not found");
        CHECK(spec_ram.IsScalar(), "Field \"ram\" is not a single value");

        CHECK(spec_clock_rate, "Field \"clock-rate\" not found");
        CHECK(spec_clock_rate.IsScalar(),
              "Field \"clock-rate\" is not a single value");

        CHECK(spec_cores_count, "Field \"cores-count\" not found");
        CHECK(spec_cores_count.IsScalar(),
              "Field \"cores-count\" is not a single value");

        auto name = spec_name.as<std::string>();

        infra::ServerSpec server_spec{};
        server_spec.ram = spec_ram.as<types::RAMBytes>();
        // TODO: write in a normal way
        server_spec.clock_rate = static_cast<types::CPUHertz>(
            spec_clock_rate.as<float>() * 1'000'000);
        server_spec.cores_count = spec_cores_count.as<uint32_t>();

        auto it = server_specs_.find(name);
        CHECK(it == server_specs_.end(), "Name {} is already in use", name);

        server_specs_.emplace(name, server_spec);
        servers_count_[name] = 0;
    }
}

void
sim::core::SimulatorConfig::ParseCloud(
    const std::string& cloud_file_name, sim::types::UUID cloud_handle,
    ActorRegister* actor_register,
    ServerSchedulerManager* server_scheduler_manager)
{
    auto cloud_config = YAML::LoadFile(cloud_file_name);

    auto dc_configs = cloud_config["data-centers"];

    CHECK(dc_configs, "Field \"data-centers\" not found");
    CHECK(dc_configs.IsSequence(), "\"data-centers\" is not a sequence");

    auto cloud = actor_register->GetActor<infra::Cloud>(cloud_handle);

    for (const auto& dc_config : dc_configs) {
        auto name_config = dc_config["name"];
        auto servers_config = dc_config["servers"];

        CHECK(name_config, "Field \"name\" not found");
        CHECK(name_config.IsScalar(), "Field \"name\" is not a single value");

        CHECK(servers_config, "Field \"servers\" not found");
        CHECK(servers_config.IsSequence(), "\"servers\" is not a sequence");

        auto data_center = actor_register->Make<infra::DataCenter>(
            name_config.as<std::string>());

        cloud->AddDataCenter(data_center->UUID());

        for (const auto& server_config : servers_config) {
            auto server_name_config = server_config["name"];
            auto server_count_config = server_config["count"];
            auto server_scheduler_config = server_config["scheduler"];

            CHECK(server_name_config, "Field \"name\" not found");
            CHECK(server_name_config.IsScalar(),
                  "Field \"name\" is not a single value");

            CHECK(server_count_config, "Field \"count\" not found");
            CHECK(server_count_config.IsScalar(),
                  "Field \"count\" is not a single value");

            CHECK(server_scheduler_config, "Field \"scheduler\" not found");
            CHECK(server_scheduler_config.IsScalar(),
                  "Field \"scheduler\" is not a single value");

            auto server_name = server_name_config.as<std::string>();
            auto servers_count = server_count_config.as<uint32_t>();
            auto server_scheduler = server_scheduler_config.as<std::string>();
            auto it = server_specs_.find(server_name);

            CHECK(it != server_specs_.end(),
                  "Server name {} is not found in specs", server_name);

            auto& server_spec = it->second;

            for (uint32_t i = 1; i <= servers_count; ++i) {
                auto& serial = servers_count_[server_name];

                auto server = actor_register->Make<infra::Server>(
                    server_name + "-" + std::to_string(++serial));

                server->SetSpec(server_spec);

                CHECK(server_scheduler == "greedy",
                      "Unknown server scheduler {}", server_scheduler);

                server_scheduler_manager->Make<custom::GreedyServerScheduler>(
                    server->UUID());

                data_center->AddServer(server->UUID());
            }
        }
    }
}
