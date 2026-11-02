#include <iostream>
#include <cassert>
#include "order_book.hpp"

void sequentialTest(){
    OrderBook book;

    book.addOrder({1, Side::Buy, 100, 10});
    book.addOrder({2, Side::Buy, 101, 5});
    book.addOrder({3, Side::Buy, 100, 7});
    book.addOrder({4, Side::Buy, 101, 13});

    const Order* bestBid = book.bestBid();

    assert(bestBid != nullptr);
    assert(bestBid->id == 2);
    assert(bestBid->price == 101);

    book.addOrder({5, Side::Sell, 105, 8});
    book.addOrder({6, Side::Sell, 103, 6});
    book.addOrder({7, Side::Sell, 105, 4});

    const Order* bestAsk = book.bestAsk();
    
    assert(bestAsk != nullptr);
    assert(bestAsk->id == 6);
    assert(bestAsk->price == 103);
}

void emptyBookTest(){
    OrderBook book;
    assert(book.bestBid() == nullptr);
    assert(book.bestAsk() == nullptr);
}

int main(){
    sequentialTest();
    emptyBookTest();

    std::cout << "All tests passed \n";
}