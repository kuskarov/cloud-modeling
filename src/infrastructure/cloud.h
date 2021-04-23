#pragma once

#include "data-center.h"
#include "resource.h"

namespace sim::infra {

class Cloud : public IResource
{
 public:
    Cloud() : IResource("Cloud") { SetName("cloud"); }

    // for scheduler
    [[nodiscard]] const auto& DataCenters() const { return data_centers_; }

    types::EnergyCount SpentPower() override { return 0; }

    void AddDataCenter(std::shared_ptr<infra::DataCenter> data_center)
    {
        data_center->SetOwner(this);
        data_center->SetScheduleFunction(schedule_event);

        data_centers_.push_back(std::move(data_center));
    }

 private:
    std::vector<std::shared_ptr<infra::DataCenter>> data_centers_{};

    void StartBoot(const ResourceEvent* resource_event) override;
    void StartShutdown(const ResourceEvent* resource_event) override;
};

}   // namespace sim::infra
