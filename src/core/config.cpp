#include "config.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <argparse.hpp>

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

    // throws std::runtime_error on fail, it is handled in the upper scope
    parser.parse_args(argc, argv);

    config_path_ = parser.get<std::string>("--config");
}

#define CHECK(condition, ...)                                   \
    {                                                           \
        if (!(condition)) {                                     \
            SimulatorLogger().PrintErrorToConsole(__VA_ARGS__); \
            throw std::runtime_error("Parse error");            \
        }                                                       \
    }

void
sim::core::SimulatorConfig::ParseResources(
    const std::shared_ptr<infra::Cloud>& cloud,
    const std::shared_ptr<infra::VMStorage>& vm_storage)
{
    ParseSpecs(config_path_ + "/specs.yaml");
    ParseCloud(config_path_ + "/cloud.yaml", cloud, vm_storage);
}

void
sim::core::SimulatorConfig::ParseSpecs(const std::string& specs_file_name)
{
    CHECK(FileExists(specs_file_name), "File {} does not exist",
          specs_file_name);

    YAML::Node specs = YAML::LoadFile(specs_file_name);

    CHECK(specs.IsSequence(), "File {} is not a sequence", specs_file_name);

    for (const auto& spec : specs) {
        infra::Server server_template{};

        auto spec_name = spec["name"];
        auto spec_ram = spec["ram"];
        auto spec_clock_rate = spec["clock-rate"];
        auto spec_cores_count = spec["cores-count"];
        auto spec_cost = spec["cost"];

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

        CHECK(spec_cost, "Field \"cost\" not found");
        CHECK(spec_cost.IsScalar(), "Field \"cost\" is not a single value");

        server_template.SetName(spec_name.as<std::string>());
        server_template.SetRam(spec_ram.as<types::RAMBytes>());
        // TODO: write in a normal way
        server_template.SetClockRate(static_cast<types::CPUHertz>(
            spec_clock_rate.as<float>() * 1'000'000));
        server_template.SetCoresCount(spec_cores_count.as<uint32_t>());
        server_template.SetCost(spec_cost.as<types::Currency>());

        auto it = server_specs_.find(server_template.GetName());
        CHECK(it == server_specs_.end(), "Name {} is already in use",
              server_template.GetName());

        server_specs_.emplace(server_template.GetName(),
                              std::move(server_template));
    }
}

void
sim::core::SimulatorConfig::ParseCloud(
    const std::string& cloud_file_name,
    const std::shared_ptr<infra::Cloud>& cloud,
    const std::shared_ptr<infra::VMStorage>& vm_storage)
{
    auto cloud_config = YAML::LoadFile(cloud_file_name);

    auto dc_configs = cloud_config["data-centers"];

    CHECK(dc_configs, "Field \"data-centers\" not found");
    CHECK(dc_configs.IsSequence(), "\"data-centers\" is not a sequence");

    for (const auto& dc_config : dc_configs) {
        auto data_center = std::make_shared<infra::DataCenter>();

        cloud->AddDataCenter(data_center);

        auto name_config = dc_config["name"];
        auto servers_config = dc_config["servers"];

        CHECK(name_config, "Field \"name\" not found");
        CHECK(name_config.IsScalar(), "Field \"name\" is not a single value");

        CHECK(servers_config, "Field \"servers\" not found");
        CHECK(servers_config.IsSequence(), "\"servers\" is not a sequence");

        data_center->SetName(name_config.as<std::string>());

        for (const auto& server_config : servers_config) {
            auto server_name_config = server_config["name"];
            auto server_count_config = server_config["count"];

            CHECK(server_name_config, "Field \"name\" not found");
            CHECK(server_name_config.IsScalar(),
                  "Field \"name\" is not a single value");

            CHECK(server_count_config, "Field \"count\" not found");
            CHECK(server_count_config.IsScalar(),
                  "Field \"count\" is not a single value");

            auto server_name = server_name_config.as<std::string>();
            auto servers_count = server_count_config.as<uint32_t>();
            auto it = server_specs_.find(server_name);

            CHECK(it != server_specs_.end(),
                  "Server name {} is not found in specs", server_name);

            auto& generator = it->second;

            data_center->AddServers(generator, servers_count);
        }
    }
}
