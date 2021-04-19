#pragma once

#include "actor.h"
#include "resource.h"
#include "server.h"

namespace sim::infra {

struct DataCenterEvent : events::Event
{
};

class DataCenter : public infra::Resource
{
 public:
    DataCenter() : infra::Resource("Data-Center") {}

    void AddServer(const std::shared_ptr<Server>& server)
    {
        servers_.push_back(server);
    }

    void HandleEvent(const std::shared_ptr<events::Event>& event) override;

    [[nodiscard]] uint32_t ServersCount() const { return servers_.size(); }

 private:
    std::vector<std::shared_ptr<Server>> servers_{};
};

}   // namespace sim::infra
