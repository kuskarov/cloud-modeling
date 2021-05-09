#pragma once

#include <string>
#include <unordered_map>

#include "actor-register.h"
#include "server.h"

namespace sim::core {

class SimulatorConfig
{
 public:
    SimulatorConfig() : whoami_("Config") {}

    void ParseArgs(int argc, char** argv);
    void ParseResources(types::UUID cloud_handle,
                        ActorRegister* actor_register);

 private:
    std::string whoami_{};

    void ParseSpecs(const std::string& specs_file_name);
    void ParseCloud(const std::string& cloud_file_name,
                    types::UUID cloud_handle, ActorRegister* actor_register);

    std::string config_path_{};

    std::unordered_map<std::string, infra::ServerSpec> server_specs_{};
    std::unordered_map<std::string, uint32_t> servers_count_{};
};

}   // namespace sim::core
