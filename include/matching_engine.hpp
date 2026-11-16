#pragma once

#include <thread>
#include <atomic>

#include "order_book.hpp"
#include "order_command.hpp"
#include "spsc_queue.hpp"

class MatchingEngine{
private:
    OrderBook book_;
    AtomicRingBuffer<OrderCommand> inputQueue_;
    std::thread worker_;
    std::atomic<bool> running_;

    void run(){
        OrderCommand command;

        while(true){

            if(inputQueue_.pop(command)){

                if(command.type == OrderType::NewOrder){
                    book_.processOrder(command.order);
                } else if(command.type == OrderType::CancelOrder){
                    book_.cancelOrder(command.orderId);
                } else if(command.type == OrderType::ModifyOrder){
                    book_.modifyOrder(command.orderId, command.newPrice, command.newQuantity);
                }

                continue;
            }

            if(!running_){
                break;
            }
        }
    }

public:
    explicit MatchingEngine(std::size_t queueCapacity) 
        : inputQueue_(queueCapacity),
          running_(false)
    {}

    ~MatchingEngine(){
        stop();
    }

    void start(){
        if(running_)
            return;

        running_ = true;
        worker_ = std::thread(&MatchingEngine::run, this);
    }
    
    void stop(){
        if(!running_)
            return;

        running_ = false;

        if(worker_.joinable()){
            worker_.join();
        }
    }

    bool submit(const OrderCommand& command){
        if(running_){
            return inputQueue_.push(command);
        }
        return false;
    }
};