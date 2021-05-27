#pragma once

#include <random>

namespace sim::custom {

class RandomUniformWorkloadModel : public infra::IVMWorkloadModel
{
 public:
    RandomUniformWorkloadModel() : generator(time(nullptr)) {}

    void Setup(
        const std::unordered_map<std::string, std::string>& params) override
    {
        if (auto it = params.find("required_ram"); it != params.end()) {
            required_ram_ =
                RAMBytes{static_cast<uint32_t>(std::stoi(it->second))};

            ram_distribution =
                std::uniform_int_distribution<uint32_t>{0, required_ram_.get()};
        } else {
            throw std::invalid_argument("required_ram field not found");
        }

        if (auto it = params.find("required_cpu"); it != params.end()) {
            required_cpu_ = CPUUtilizationPercent{
                static_cast<uint32_t>(std::stoi(it->second))};

            cpu_distribution =
                std::uniform_int_distribution<uint32_t>{0, required_cpu_.get()};
        } else {
            throw std::invalid_argument("required_cpu field not found");
        }

        if (auto it = params.find("required_bandwidth"); it != params.end()) {
            required_bandwidth_ =
                IOBandwidthMBpS{static_cast<uint32_t>(std::stoi(it->second))};

            bw_distribution = std::uniform_int_distribution<uint32_t>{
                0, required_bandwidth_.get()};
        } else {
            throw std::invalid_argument("required_bandwidth field not found");
        }
    }

    infra::Workload GetWorkload(TimeStamp time) override
    {
        return {RAMBytes{ram_distribution(generator)},
                CPUUtilizationPercent{cpu_distribution(generator)},
                IOBandwidthMBpS{bw_distribution(generator)}};
    }

 private:
    RAMBytes required_ram_{};
    CPUUtilizationPercent required_cpu_{};
    IOBandwidthMBpS required_bandwidth_{};

    std::uniform_int_distribution<uint32_t> ram_distribution, cpu_distribution,
        bw_distribution;

    std::mt19937 generator;
};

}   // namespace sim::custom
