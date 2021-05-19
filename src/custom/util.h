#pragma once

namespace sim::custom {

typedef std::function<IScheduler*()> SchedulerCreator;

template <typename Scheduler>
static SchedulerCreator
MakeScheduler()
{
    static_assert(std::is_base_of_v<IScheduler, Scheduler>);

    return []() -> IScheduler* { return new Scheduler{}; };
}

typedef std::function<infra::IVMWorkload*()> WorkloadCreator;

template <typename Workload>
static WorkloadCreator
MakeWorkload()
{
    static_assert(std::is_base_of_v<infra::IVMWorkload, Workload>);

    return []() -> infra::IVMWorkload* { return new Workload{}; };
}

}   // namespace sim::custom
