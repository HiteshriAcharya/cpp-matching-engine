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
    auto result = book.processOrder({2, Side::Buy, 105,6});

    assert(result.success == true);
    assert(result.trades.size() == 1);
    assert(result.trades[0].buyOrderId == 2);
    assert(result.trades[0].sellOrderId == 1);
    assert(result.trades[0].price == 100);
    assert(result.trades[0].quantity == 6);

    const Order* bestAsk = book.bestAsk();

    assert(bestAsk != nullptr);
    assert(bestAsk->id == 1);
    assert(bestAsk->price == 100);
    assert(bestAsk->quantity == 4);
}

void testIncomingOrderRestsAfterPartialFill(){
    OrderBook book;

    book.addOrder({1, Side::Sell, 100, 5});
    auto result = book.processOrder({2, Side::Buy, 105, 10});

    assert(result.success == true);
    assert(result.trades.size() == 1);
    assert(result.trades[0].quantity == 5);

    assert(book.bestAsk() == nullptr);
    assert(book.bestBid()->id == 2);
    assert(book.bestBid()->price == 105);
    assert(book.bestBid()->quantity == 5);
}

void testFullMatch(){

    OrderBook book;

    book.addOrder({1, Side::Buy, 98, 11});
    auto result = book.processOrder({2, Side::Sell, 98, 11});

    assert(result.success == true);
    assert(result.trades.size() == 1);
    assert(result.trades[0].quantity == 11);
    assert(result.trades[0].price == 98);

    assert(book.bestAsk() == nullptr);
    assert(book.bestBid() == nullptr);
}

void testMultiFill(){
    OrderBook book;

    book.addOrder({1, Side::Sell, 100, 3});
    book.addOrder({2, Side::Sell, 100, 4});
    book.addOrder({3, Side::Sell, 101, 5});

    auto result = book.processOrder({4, Side::Buy, 105, 10});

    assert(result.success == true);
    assert(result.trades.size() == 3);

    assert(result.trades[0].buyOrderId == 4);
    assert(result.trades[0].sellOrderId == 1);
    assert(result.trades[0].price == 100);
    assert(result.trades[0].quantity == 3);

    assert(result.trades[1].buyOrderId == 4);
    assert(result.trades[1].sellOrderId == 2);
    assert(result.trades[1].price == 100);
    assert(result.trades[1].quantity == 4);

    assert(result.trades[2].buyOrderId == 4);
    assert(result.trades[2].sellOrderId == 3);
    assert(result.trades[2].price == 101);
    assert(result.trades[2].quantity == 3);

    assert(book.bestAsk() != nullptr);
    assert(book.bestAsk()->id == 3);
    assert(book.bestAsk()->quantity == 2);
    assert(book.bestAsk()->price == 101);

    assert(book.bestBid() == nullptr);
}

void testCancelBid(){
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

    assert(book.bestBid() != nullptr);
    assert(book.bestBid()->id == 3);
}

void testModifyDecreaseQuantityKeepsPriority(){
    OrderBook book;

    book.addOrder({1, Side::Sell, 100, 5});
    book.addOrder({2, Side::Sell, 100, 10});
    book.addOrder({3, Side::Sell, 100, 30});

    ModifyResult result = book.modifyOrder(1, 100, 2);

    assert(result.success == true);
    assert(result.trades.empty() == true);

    assert(book.bestAsk()->id == 1);
    assert(book.bestAsk()->quantity == 2);
}

void testModifyIncreaseQuantityLosesPriority(){
    OrderBook book;

    book.addOrder({1, Side::Sell, 100, 5});
    book.addOrder({2, Side::Sell, 100, 10});
    book.addOrder({3, Side::Sell, 100, 30});

    ModifyResult result = book.modifyOrder(1, 100, 90);

    assert(result.success == true);
    assert(result.trades.empty() == true);

    assert(book.bestAsk()->id == 2);
    assert(book.bestAsk()->quantity == 10);
}

void testModifyPriceChangeMovesOrder(){
    OrderBook book;

    book.addOrder({1, Side::Sell, 100, 5});
    book.addOrder({2, Side::Sell, 100, 10});
    book.addOrder({3, Side::Sell, 100, 30});
    
    ModifyResult result = book.modifyOrder(1, 140, 90);

    assert(result.success == true);
    assert(result.trades.empty() == true);

    assert(book.bestAsk()->id == 2);
    assert(book.bestAsk()->quantity == 10);

    assert(book.cancelOrder(1) == true);
}

void testModifyPriceChangeTriggersTrade(){
    OrderBook book;

    book.addOrder({1, Side::Buy, 100, 5});
    book.addOrder({10, Side::Sell, 105, 5});

    ModifyResult result = book.modifyOrder(1, 110, 5);

    assert(result.success == true);
    assert(result.trades.size() == 1);

    assert(result.trades.at(0).price == 105);
    assert(result.trades.at(0).quantity == 5);
    assert(result.trades.at(0).buyOrderId == 1);
    assert(result.trades.at(0).sellOrderId == 10);


    assert(book.bestAsk() == nullptr);
    assert(book.bestBid() == nullptr);
}

void testModifyZeroQuantityCancelsOrder(){
    OrderBook book;

    book.addOrder({1, Side::Buy, 100, 10});

    ModifyResult result = book.modifyOrder(1, 100, 0);

    assert(result.success == true);
    assert(result.trades.empty());
    assert(book.bestBid() == nullptr);
}

void testModifyNonExistentOrder(){
    OrderBook book;

    ModifyResult result = book.modifyOrder(999, 100, 10);

    assert(result.success == false);
    assert(result.trades.empty());
}

void testDuplicateProcessOrderRejected(){
    OrderBook book;

    assert(book.addOrder({1, Side::Buy, 100, 10}) == true);

    ProcessResult result =
        book.processOrder({1, Side::Sell, 105, 5});

    assert(result.success == false);
    assert(result.trades.empty());

    assert(book.bestBid() != nullptr);
    assert(book.bestBid()->id == 1);
    assert(book.bestBid()->price == 100);
}

void testDuplicateAddOrderRejected(){
    OrderBook book;

    assert(book.addOrder({1, Side::Buy, 100, 10}) == true);
    assert(book.addOrder({1, Side::Buy, 101, 20}) == false);

    assert(book.bestBid() != nullptr);
    assert(book.bestBid()->id == 1);
    assert(book.bestBid()->price == 100);
    assert(book.bestBid()->quantity == 10);
}

int main(){
    sequentialTest();
    emptyBookTest();
    testPartialFillRestingAsk();
    testIncomingOrderRestsAfterPartialFill();
    testFullMatch();
    testMultiFill();
    testCancelBid();
    testCancelNonExistentOrder();
    testCancelMiddleOrderAtSamePrice();
    testModifyDecreaseQuantityKeepsPriority();
    testModifyIncreaseQuantityLosesPriority();
    testModifyPriceChangeMovesOrder();
    testModifyPriceChangeTriggersTrade();
    testModifyZeroQuantityCancelsOrder();
    testModifyNonExistentOrder();
    testDuplicateProcessOrderRejected();
    testDuplicateAddOrderRejected();

    std::cout << "All tests passed \n";
}