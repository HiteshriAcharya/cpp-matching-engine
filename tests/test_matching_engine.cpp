#include <iostream>
#include <cassert>
#include "matching_engine.hpp"

void testMatchingEngine(){
    MatchingEngine engine(16);

    engine.start();

    OrderCommand command;
    command.type = OrderType::NewOrder;
    command.order = {1, Side::Buy, 100, 5};

    assert(engine.submit(command));

    engine.stop();
}

int main(){
    testMatchingEngine();

    std::cout << "MatchingEngine tests passed\n";
}