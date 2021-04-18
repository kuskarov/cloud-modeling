#include "data-center.h"

#include <loguru.hpp>

void
sim::resources::DataCenter::HandleEvent(
    const std::shared_ptr<events::Event>& event)
{
    try {
        auto dc_event = dynamic_cast<const DataCenterEvent*>(event.get());
        LOG_F(INFO, "Data center event!");

        for (const auto& server : servers_) {
            auto startup_server = std::make_shared<ResourceEvent>();
            startup_server->resource_event_type = ResourceEventType::kBoot;
            startup_server->addressee = server.get();

            schedule_callback_(event->happen_ts, startup_server, false);
        }

        LOG_F(INFO, "Scheduled servers' startup!");
    } catch (const std::bad_cast& bc) {
        LOG_F(ERROR, "Data-center %s received non-server event!",
              name_.c_str());
    }
}
