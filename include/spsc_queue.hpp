#pragma once
#include <atomic>
#include <memory>
#include <stdexcept>

template<typename T>
class AtomicRingBuffer{
private:
    std::size_t capacity_;
    std::unique_ptr<T[]> buffer_;
    
    alignas(64) std::atomic<std::size_t> head_;
    alignas(64) std::atomic<std::size_t> tail_;

public:
    explicit AtomicRingBuffer(std::size_t capacity) : capacity_(capacity), 
                                                      buffer_(nullptr),
                                                      head_(0),
                                                      tail_(0)
    {
        if(capacity_ <= 1){
            throw std::invalid_argument("AtomicRingBuffer capacity_ must be at least 2");
        }
        buffer_ = std::make_unique<T[]>(capacity_); 
    }
    
    AtomicRingBuffer(const AtomicRingBuffer& other) = delete;
    AtomicRingBuffer& operator=(const AtomicRingBuffer& other) = delete;
    AtomicRingBuffer(AtomicRingBuffer&& other) = delete;
    AtomicRingBuffer& operator=(AtomicRingBuffer&& other) = delete;
    ~AtomicRingBuffer() = default;

    bool push(const T& data){
        std::size_t currTail = tail_.load(std::memory_order_relaxed);
        std::size_t currHead = head_.load(std::memory_order_acquire);

        if((currTail + 1) % capacity_ == currHead){
            return false;
        }

        buffer_[currTail] = data;
        tail_.store((currTail + 1) % capacity_, std::memory_order_release);

        return true;
    }

    bool pop(T& data){
        std::size_t currHead = head_.load(std::memory_order_relaxed);
        std::size_t currTail = tail_.load(std::memory_order_acquire);

        if(currHead == currTail){
            return false;
        }

        data = buffer_[currHead];

        head_.store((currHead + 1) % capacity_, std::memory_order_release);

        return true;
    }
};