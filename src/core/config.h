#pragma once

#include <string>
#include <unordered_map>

#include "actor-register.h"
#include "resource-scheduler.h"
#include "server.h"

namespace sim::core {

class SimulatorConfig
{
 public:
    SimulatorConfig() : whoami_("Config") {}

    void ParseArgs(int argc, char** argv);
    void ParseResources(UUID cloud_handle,
                        events::ActorRegister* actor_register,
                        ServerSchedulerManager* server_scheduler_manager);

    auto GetLogsPath() const { return logs_path_; }
    auto GetPort() const { return port_; }

    std::string_view WhoAmI() const { return whoami_; }

 private:
    std::string whoami_{};

    void ParseSpecs(const std::string& specs_file_name);
    void ParseCloud(const std::string& cloud_file_name, UUID cloud_handle,
                    events::ActorRegister* actor_register,
                    ServerSchedulerManager* server_scheduler_manager);

    std::string config_path_{}, logs_path_{};
    uint32_t port_{};

    std::unordered_map<std::string, infra::ServerSpec> server_specs_{};
    std::unordered_map<std::string, uint32_t> servers_count_{};
};

}   // namespace sim::core
