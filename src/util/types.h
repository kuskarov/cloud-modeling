#pragma once

#include <fmt/format.h>

#include <cstdint>
#include <NamedType/named_type.hpp>

namespace sim::types {

using fluent::Addable;
using fluent::Comparable;
using fluent::NamedType;
using fluent::Subtractable;

// TODO: strong typedef

using EnergyCount = NamedType<uint64_t, struct EnergyCountTag>;

typedef int64_t TimeStamp;

typedef int64_t TimeInterval;

using RAMBytes =
    NamedType<uint64_t, struct RAMBytesTag, Comparable, Addable, Subtractable>;

using CPUHertz = NamedType<uint64_t, struct CPUHertzTag>;

class UUID
{
 public:
    UUID() = default;

    UUID(const UUID& other) = default;

    UUID& operator=(const UUID& other) = default;

    UUID(UUID&& other) = default;

    // UUID(UUID&& other) = default;

    static UUID Generate()
    {
        static uint32_t serial{1};
        return UUID{serial++};
    }

    bool operator==(const UUID& other) const { return value_ == other.value_; }

    explicit operator bool() const { return value_ != 0; }

    explicit operator uint32_t() const { return value_; }

 private:
    explicit UUID(uint32_t value) : value_(value) {}

    uint32_t value_{};
};

}   // namespace sim::types

template <>
struct fmt::formatter<sim::types::UUID>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const sim::types::UUID& uuid, FormatContext& ctx)
    {
        return format_to(ctx.out(), "{}", static_cast<uint32_t>(uuid));
    }
};

template <>
struct std::hash<sim::types::UUID>
{
    size_t operator()(const sim::types::UUID& uuid) const
    {
        return hash<uint32_t>()(static_cast<uint32_t>(uuid));
    }
};
