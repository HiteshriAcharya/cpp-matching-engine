#pragma once

#include <map>
#include<unordered_map>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>

#include "order.hpp"
#include "trade.hpp"

class OrderBook{
private:

    struct OrderLocation{
        Side side;
        std::uint64_t price;
        std::list<Order>::iterator it;
    };

    std::map<std::uint64_t, std::list<Order>, std::greater<std::uint64_t>> bids_; 
    std::map<std::uint64_t, std::list<Order>> asks_;
    std::unordered_map<std::uint64_t, OrderLocation> orderIndex_; 

    std::vector<Trade> matchBuyOrder(Order& order){
        std::vector<Trade> trades;

        while(order.quantity > 0 
            && !asks_.empty() 
            && order.price >= bestAsk()->price)
        {
            auto askLevel = asks_.begin();
            Order& restingAsk = askLevel->second.front();

            std::uint32_t tradeQty = std::min(order.quantity, restingAsk.quantity);

            trades.push_back({
                order.id,
                restingAsk.id,
                restingAsk.price,
                tradeQty
            });

            order.quantity -= tradeQty;
            restingAsk.quantity -= tradeQty;

            if(restingAsk.quantity == 0){
                orderIndex_.erase(restingAsk.id);
                askLevel->second.pop_front();
            }

            if(askLevel->second.empty()){
                asks_.erase(askLevel);
            }
        }

        if(order.quantity > 0){
            bids_[order.price].push_back(order);
            auto it = std::prev(bids_[order.price].end());
            orderIndex_[order.id] = {Side::Buy, order.price, it};
        }

        return trades;
    }

    std::vector<Trade> matchSellOrder(Order& order){
        std::vector<Trade> trades;

        while(order.quantity > 0 
            && !bids_.empty() 
            && order.price <= bestBid()->price)
        {
            auto bidLevel = bids_.begin();
            Order& restingBid = bidLevel->second.front();

            std::uint32_t tradeQty = std::min(order.quantity, restingBid.quantity);

            trades.push_back({
                restingBid.id,
                order.id,
                restingBid.price,
                tradeQty
            });

            order.quantity -= tradeQty;
            restingBid.quantity -= tradeQty;

            if(restingBid.quantity == 0){
                orderIndex_.erase(restingBid.id);
                bidLevel->second.pop_front();
            }

            if(bidLevel->second.empty()){
                bids_.erase(bidLevel);
            }
        }

        if(order.quantity > 0){
            asks_[order.price].push_back(order);
            auto it = std::prev(asks_[order.price].end());
            orderIndex_[order.id] = {Side::Sell, order.price, it};
        }
        
        return trades;
    }

    bool cancelFromBook(auto& book, std::uint64_t orderId) {
        auto indexIt = orderIndex_.find(orderId);

        auto levelIt = book.find(indexIt->second.price);

        levelIt->second.erase(indexIt->second.it);
        orderIndex_.erase(indexIt);

        if(levelIt->second.empty()){
            book.erase(levelIt);
        }
        return true;
    }

    ModifyResult modifyOrderFromBook(auto& book, 
                                     std::uint64_t orderId, 
                                     std::uint64_t newPrice, 
                                     std::uint32_t newQuantity)
    {
        auto indexIt = orderIndex_.find(orderId);

        Order& existingOrder = *(indexIt->second.it);

        if(existingOrder.price == newPrice 
            && newQuantity <= existingOrder.quantity)
        {
            existingOrder.quantity = newQuantity;
            return {true, {}};
        }

        Order updatedOrder = existingOrder;
        updatedOrder.price = newPrice;
        updatedOrder.quantity = newQuantity;

        cancelFromBook(book, orderId);

        return {true, processOrder(updatedOrder)};
    }

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
            auto it = std::prev(bids_[order.price].end());
            orderIndex_[order.id] = {Side::Buy, order.price, it};
        } else if(order.side == Side::Sell){
            asks_[order.price].push_back(order);
            auto it = std::prev(asks_[order.price].end());
            orderIndex_[order.id] = {Side::Sell, order.price, it};
        }
    }

    const Order* bestBid() const{

        if(bids_.empty())
            return nullptr;
        
        auto it = bids_.begin();
        return &it->second.front();
    }

    const Order* bestAsk() const{

        if(asks_.empty())
            return nullptr;

        auto it = asks_.begin();
        return &it->second.front();
    }

    std::vector<Trade> processOrder(Order order){
        if(order.side == Side::Buy){
            return matchBuyOrder(order);
        }
        return matchSellOrder(order);
    }

    bool cancelOrder(std::uint64_t orderId) {
        auto it = orderIndex_.find(orderId);
        
        if(it == orderIndex_.end())
            return false;
        
        if(it->second.side == Side::Buy){
            return cancelFromBook(bids_, orderId);
        }
        return cancelFromBook(asks_, orderId);
    }

    ModifyResult modifyOrder(std::uint64_t orderId, 
                     std::uint64_t newPrice, 
                     std::uint32_t newQuantity)
    {
        if(newQuantity == 0){
            return {cancelOrder(orderId), {}};
        }

        auto indexIt = orderIndex_.find(orderId);

        if(indexIt == orderIndex_.end())
            return {false, {}};

        if(indexIt->second.side == Side::Buy){
            return modifyOrderFromBook(bids_, orderId, newPrice, newQuantity);
        }
        return modifyOrderFromBook(asks_, orderId, newPrice, newQuantity);
    }
};