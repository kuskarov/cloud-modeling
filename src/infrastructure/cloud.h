#pragma once

#include "data-center.h"
#include "resource.h"

namespace sim::infra {

class Cloud : public Resource
{
 public:
    Cloud() : Resource("Cloud") {}

    void HandleEvent(const std::shared_ptr<events::Event>& event) override;

    const auto& DataCenters() { return data_centers_; }

    void AddDataCenter(std::shared_ptr<infra::DataCenter> data_center)
    {
        data_centers_.push_back(std::move(data_center));
    }

 private:
    std::vector<std::shared_ptr<infra::DataCenter>> data_centers_{};
};

}   // namespace sim::infra
