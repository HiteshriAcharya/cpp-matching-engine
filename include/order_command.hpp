#pragma once
#include <cstdint>
#include "order.hpp"

enum class OrderType{
    NewOrder,
    CancelOrder,
    ModifyOrder
};

struct OrderCommand{
    OrderType type;

    Order order;
    
    std::uint64_t orderId = 0;
    std::uint64_t newPrice = 0;
    std::uint32_t newQuantity = 0;
};