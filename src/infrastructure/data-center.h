#pragma once

#include "actor.h"
#include "resource.h"
#include "server.h"

namespace sim::resources {

struct DataCenterEvent : events::Event
{
};

class DataCenter : public resources::Resource
{
 public:
    void AddServer(const std::shared_ptr<Server>& server)
    {
        servers_.push_back(server);
    }

    // TODO: need more elegant solution
    void SetServerScheduleCallback(
        const std::function<void(types::TimeStamp,
                                 const std::shared_ptr<events::Event>&, bool)>&
            schedule_callback)
    {
        for (auto& server : servers_) {
            server->SetScheduleCallback(schedule_callback);
        }
    }

    void HandleEvent(const std::shared_ptr<events::Event>& event) override;

    [[nodiscard]] uint32_t ServersCount() const { return servers_.size(); }

 private:
    std::vector<std::shared_ptr<Server>> servers_{};
};

}   // namespace sim::resources
