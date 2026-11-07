#pragma once
#include <cstdint>

enum class Side{
    Buy,
    Sell,
    Cancel
};

struct Order{
    std::uint64_t id;
    Side side;
    std::uint64_t price;
    std::uint32_t quantity;
};