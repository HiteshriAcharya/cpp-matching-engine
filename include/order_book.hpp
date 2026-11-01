#pragma once
#include <map>
#include <deque>
#include <functional>
#include "order.hpp"

class OrderBook{
private:
    std::map<std::uint64_t, std::deque<Order>, std::greater<std::uint64_t>> bids_; 
    std::map<std::uint64_t, std::deque<Order>> asks_;

public:

    OrderBook() = default;

    OrderBook(const OrderBook& other) = delete;
    OrderBook& operator=(const OrderBook& other) = delete;
    OrderBook(OrderBook&& other) = delete;
    OrderBook& operator=(OrderBook&& other) = delete;

    ~OrderBook() = default;

    void addOrder(const Order& order){
        if(order.side == Side::Buy){
            bids_[order.price].push_back(order);
        } else{
            asks_[order.price].push_back(order);
        }
    }
};