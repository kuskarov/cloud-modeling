#pragma once

#include "data-center.h"
#include "resource.h"

namespace sim::infra {

class Cloud : public Resource
{
 public:
    Cloud() : Resource("Cloud") { name_ = "Cloud"; }

    // for scheduler
    [[nodiscard]] const auto& DataCenters() const { return data_centers_; }

    void AddDataCenter(std::shared_ptr<infra::DataCenter> data_center)
    {
        data_centers_.push_back(std::move(data_center));
    }

 private:
    std::vector<std::shared_ptr<infra::DataCenter>> data_centers_{};

    void StartBoot(const ResourceEvent* resource_event) override;
};

}   // namespace sim::infra
