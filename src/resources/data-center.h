#pragma once

#include "actor.h"
#include "resource.h"
#include "server.h"

namespace sim::resources {

class DataCenter : public sim::resources::Resource
{
 public:
    void AddServer(const Server &server) { servers_.push_back(server); }

    void AddServers(const Server &server, uint32_t count)
    {
        servers_.insert(servers_.end(), count, server);
    }

    // TODO: need more elegant solution
    void SetServerScheduleCallback(
        const std::function<void(sim::types::TimeStamp, events::Event &&, bool)>
            &schedule_callback)
    {
        for (auto &server : servers_) {
            server.SetScheduleCallback(schedule_callback);
        }
    }

 private:
    std::vector<Server> servers_{};
};

}   // namespace sim::resources
