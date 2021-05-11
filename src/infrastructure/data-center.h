#pragma once

#include <algorithm>

#include "actor.h"
#include "resource.h"
#include "server.h"

namespace sim::infra {

class DataCenter : public IResource
{
 public:
    DataCenter() : IResource("Data-Center") {}

    void AddServer(types::UUID uuid)
    {
        servers_.push_back(uuid);
        AddComponent(uuid);
    }

    types::EnergyCount SpentPower() override { return 0; }

    // for scheduler
    [[nodiscard]] const auto& Servers() const { return servers_; }

 private:
    /**
     * Separate storage for servers, will be significant when DataCenter will
     * contain network switches in additions to servers
     */
    std::vector<types::UUID> servers_{};
};

}   // namespace sim::infra
