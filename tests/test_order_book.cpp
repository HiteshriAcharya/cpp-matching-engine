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

void testPartialFillRestingAsk(){

    OrderBook book;

    book.addOrder({1, Side::Sell, 100, 10});
    auto trades = book.processOrder({2, Side::Buy, 105,6});

    assert(trades.size() == 1);
    assert(trades[0].buyOrderId == 2);
    assert(trades[0].sellOrderId == 1);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 6);

    const Order* bestAsk = book.bestAsk();

    assert(bestAsk != nullptr);
    assert(bestAsk->id == 1);
    assert(bestAsk->price == 100);
    assert(bestAsk->quantity == 4);
}

void testIncomingOrderRestsAfterPartialFill(){
    OrderBook book;

    book.addOrder({1, Side::Sell, 100, 5});
    auto trades = book.processOrder({2, Side::Buy, 105, 10});

    assert(trades.size() == 1);
    assert(trades[0].quantity == 5);

    assert(book.bestAsk() == nullptr);
    assert(book.bestBid()->id == 2);
    assert(book.bestBid()->price == 105);
    assert(book.bestBid()->quantity == 5);
}

void testFullMatch(){

    OrderBook book;

    book.addOrder({1, Side::Buy, 98, 11});
    auto trades = book.processOrder({2, Side::Sell, 98, 11});

    assert(trades.size() == 1);
    assert(trades[0].quantity == 11);
    assert(trades[0].price == 98);

    assert(book.bestAsk() == nullptr);
    assert(book.bestBid() == nullptr);
}

void testMultiFill(){
    OrderBook book;

    book.addOrder({1, Side::Sell, 100, 3});
    book.addOrder({2, Side::Sell, 100, 4});
    book.addOrder({3, Side::Sell, 101, 5});

    auto trades = book.processOrder({4, Side::Buy, 105, 10});

    assert(trades.size() == 3);

    assert(trades[0].buyOrderId == 4);
    assert(trades[0].sellOrderId == 1);
    assert(trades[0].price == 100);
    assert(trades[0].quantity == 3);

    assert(trades[1].buyOrderId == 4);
    assert(trades[1].sellOrderId == 2);
    assert(trades[1].price == 100);
    assert(trades[1].quantity == 4);

    assert(trades[2].buyOrderId == 4);
    assert(trades[2].sellOrderId == 3);
    assert(trades[2].price == 101);
    assert(trades[2].quantity == 3);

    assert(book.bestAsk() != nullptr);
    assert(book.bestAsk()->id == 3);
    assert(book.bestAsk()->quantity == 2);
    assert(book.bestAsk()->price == 101);

    assert(book.bestBid() == nullptr);
}

void testCancelAsk(){
    OrderBook book;

    book.addOrder({1, Side::Buy, 100, 15});
    book.addOrder({2, Side::Buy, 100, 5});

    bool cancelled = book.cancelOrder(2);

    assert(cancelled == true);
    assert(book.bestBid() != nullptr);
    assert(book.bestBid()->id == 1);
}

void testCancelNonExistentOrder(){
    OrderBook book;

    book.addOrder({1, Side::Buy, 100, 10});

    assert(book.cancelOrder(999) == false);
    assert(book.bestBid() != nullptr);
    assert(book.bestBid()->id == 1);
}

void testCancelMiddleOrderAtSamePrice(){
    OrderBook book;

    book.addOrder({1, Side::Buy, 100, 10});
    book.addOrder({2, Side::Buy, 100, 20});
    book.addOrder({3, Side::Buy, 100, 30});

    assert(book.cancelOrder(2) == true);

    assert(book.bestBid() != nullptr);
    assert(book.bestBid()->id == 1);

    assert(book.cancelOrder(1) == true);

    assert(book.bestBid()->id == 3);
}

int main(){
    sequentialTest();
    emptyBookTest();
    testPartialFillRestingAsk();
    testIncomingOrderRestsAfterPartialFill();
    testFullMatch();
    testMultiFill();
    testCancelAsk();
    testCancelNonExistentOrder();
    testCancelMiddleOrderAtSamePrice();

    std::cout << "All tests passed \n";
}