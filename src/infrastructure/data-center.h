#pragma once

#include "actor.h"
#include "resource.h"
#include "server.h"

namespace sim::infra {

struct DataCenterEvent : events::Event
{
};

class DataCenter : public IResource
{
 public:
    DataCenter() : IResource("Data-Center") {}

    void AddServer(const std::shared_ptr<Server>& server)
    {
        servers_.push_back(server);
    }

    types::EnergyCount SpentPower() override { return 0; }

    [[nodiscard]] uint32_t ServersCount() const { return servers_.size(); }

    // for scheduler
    [[nodiscard]] const auto& Servers() const { return servers_; }

 private:
    std::vector<std::shared_ptr<Server>> servers_{};

    void StartBoot(const ResourceEvent* resource_event) override;
};

}   // namespace sim::infra
