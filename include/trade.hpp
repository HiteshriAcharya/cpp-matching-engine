#pragma once
#include <vector>
#include <cstdint>

struct Trade{
    std::uint64_t buyOrderId;
    std::uint64_t sellOrderId;
    std::uint64_t price;
    std::uint32_t quantity;
};

struct ModifyResult{
    bool success;
    std::vector<Trade> trades;
};