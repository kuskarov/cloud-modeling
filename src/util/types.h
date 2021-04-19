#pragma once

#include <cstdint>

namespace sim::types {
// TODO: strong typedef

typedef int64_t EnergyCount;

typedef int64_t TimeStamp;

typedef int64_t TimeInterval;

typedef uint64_t UUID;

typedef uint64_t RAMBytes;

typedef uint64_t CPUHertz;

typedef uint64_t Currency;

static UUID
UniqueUUID()
{
    static UUID serial{1};
    return serial++;
}

}   // namespace sim::types
