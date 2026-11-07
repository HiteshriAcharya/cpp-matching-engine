#pragma once
#include <map>
#include <deque>
#include <vector>
#include <functional>
#include "order.hpp"
#include "trade.hpp"

class OrderBook{
private:
    std::map<std::uint64_t, std::deque<Order>, std::greater<std::uint64_t>> bids_; 
    std::map<std::uint64_t, std::deque<Order>> asks_;

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
                askLevel->second.pop_front();
            }

            if(askLevel->second.empty()){
                asks_.erase(askLevel);
            }
        }

        if(order.quantity > 0){
            bids_[order.price].push_back(order);
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
                bidLevel->second.pop_front();
            }

            if(bidLevel->second.empty()){
                bids_.erase(bidLevel);
            }
        }

        if(order.quantity > 0){
            asks_[order.price].push_back(order);
        }
        
        return trades;
    }

    bool cancelFromBook(auto& book, std::uint64_t orderId) {
        for (auto levelIt = book.begin(); levelIt != book.end(); ++levelIt) {
            auto& orders = levelIt->second;

            for (auto orderIt = orders.begin(); orderIt != orders.end(); ++orderIt) {
                if (orderIt->id == orderId) {
                    orders.erase(orderIt);

                    if (orders.empty()) {
                        book.erase(levelIt);
                    }

                    return true;
                }
            }
        }

        return false;
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
        } else if(order.side == Side::Sell){
            asks_[order.price].push_back(order);
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
        if(order.side == Side::Buy)
            return matchBuyOrder(order);
        else if(order.side == Side::Sell)
            return matchSellOrder(order);
    }

    bool cancelOrder(std::uint64_t orderId) {
        if (cancelFromBook(bids_, orderId)) {
            return true;
        }

        return cancelFromBook(asks_, orderId);
    }
};