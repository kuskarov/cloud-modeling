#pragma once

namespace sim::custom {

class ConstantVMWorkloadModel : public infra::IVMWorkloadModel
{
 public:
    void Setup(
        const std::unordered_map<std::string, std::string>& params) override
    {
        if (auto it = params.find("required_ram"); it != params.end()) {
            required_ram_ =
                RAMBytes{static_cast<uint32_t>(std::stoi(it->second))};
        } else {
            throw std::invalid_argument("required_ram field not found");
        }

        if (auto it = params.find("required_cpu"); it != params.end()) {
            required_cpu_ = CPUUtilizationPercent{
                static_cast<uint32_t>(std::stoi(it->second))};
        } else {
            throw std::invalid_argument("required_cpu field not found");
        }

        if (auto it = params.find("required_bandwidth"); it != params.end()) {
            required_bandwidth_ =
                IOBandwidthMBpS{static_cast<uint32_t>(std::stoi(it->second))};
        } else {
            throw std::invalid_argument("required_bandwidth field not found");
        }
    }

    infra::Workload GetWorkload(TimeStamp time) override
    {
        return {required_ram_, required_cpu_, required_bandwidth_};
    }

 private:
    RAMBytes required_ram_;
    CPUUtilizationPercent required_cpu_;
    IOBandwidthMBpS required_bandwidth_;
};

}   // namespace sim::custom
