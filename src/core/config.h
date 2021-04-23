#pragma once

#include <string>
#include <unordered_map>

#include "cloud.h"
#include "server.h"
#include "vm-storage.h"

namespace sim::core {

class SimulatorConfig
{
 public:
    void ParseArgs(int argc, char** argv);
    void ParseResources(const std::shared_ptr<infra::Cloud>& cloud,
                        const std::shared_ptr<infra::VMStorage>& vm_storage);

 private:
    void ParseSpecs(const std::string& specs_file_name);
    void ParseCloud(const std::string& cloud_file_name,
                    const std::shared_ptr<infra::Cloud>& cloud,
                    const std::shared_ptr<infra::VMStorage>& vm_storage);

    std::string config_path_{};

    std::unordered_map<std::string, infra::ResourceGenerator<infra::Server>>
        server_specs_{};
};

}   // namespace sim::core
