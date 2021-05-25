#pragma once

#include "scheduler.h"

namespace sim::core {

/**
 * RPC Scheduler interface
 */
class RPCScheduler : public IScheduler
{
 public:
    /**
     * Dumps cloud state, sends it via RPC and
     */
    void UpdateSchedule() override {

    }

 private:
    void DumpCloud() {}
};

}   // namespace sim::core
