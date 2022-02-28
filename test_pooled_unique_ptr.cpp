/**
 * Compile with:
 * clang++ -std=c++17 -Wall -Wextra -g -O0 -fsanitize=address test_pooled_unique_ptr.cpp
 * g++-11 -std=c++17 -Wall -Wextra -g -O0 -fsanitize=address test_pooled_unique_ptr.cpp
 * Testes with Apple Clang 13, and GNU GCC 11.
 **/

#include "pooled_unique_ptr.h"

#include <cassert>
#include <iostream>
#include <string>

struct MyObj {
    int item;
    std::string msg;
    MyObj(int t, std::string msg) : item(t), msg(msg) {}
    void greet()
    {
        std::cout << "(" << item << ") MyObj: " << msg << std::endl;
    }
};

void check_pooled_max_out()
{
    // checks that the pool is full
    // call after using all pool slots
    bool catch_exception = false;
    try {
        pooled_unique_ptr<MyObj> p5(5, "!");
    } catch (std::runtime_error& e) {
        catch_exception = true;
        std::cout << e.what() << std::endl;
    }
    assert(catch_exception && "check if the exception is thrown");
}

void test_pool_alloc_dealloc()
{
    std::cout << "*** test_pool_alloc_dealloc ***" << std::endl;
    pooled_unique_ptr<MyObj> p1(1, "hello"); // first slot
    pooled_unique_ptr<MyObj> p2(2, "world"); // second slot
    {                                        // let's create p3 in a different scope
        pooled_unique_ptr<MyObj> p3(3, "!"); // third slot
    }                                        // it goes away, third slot released
    pooled_unique_ptr<MyObj> p4(4, "!");     // forth slot
    pooled_unique_ptr<MyObj> p5(5, "!");     // used third slot - which is free
    check_pooled_max_out();                  // check that the pool max is still working
}

void test_pool_move(bool dangeruous = false)
{
    std::cout << "*** test_pool_move ***" << std::endl;

    pooled_unique_ptr<MyObj> p1(1, "hello");                // first slot
    pooled_unique_ptr<MyObj> p2 = std::move(p1);            // still first slopt
    p2->greet();                                            // should be (1) MyObj: hello (p1 data)
    // assert(p1.data() == nullptr && "p1 should be nullptr"); // p1 left unutilized
    pooled_unique_ptr<MyObj> p3(3, "!");                    // second slot
    pooled_unique_ptr<MyObj> p4;                            // not yet
    p4 = std::move(p3);                                     // second slot
    p4->greet();                                            // greets with p3 data
    assert(p4->item == 3);                                  // should be 3, from p3 data

    if (dangeruous) { // could produce a runtime error
        p1->greet();
        p3->greet();
    }

    pooled_unique_ptr<MyObj> p5 = pooled_unique_ptr<MyObj>(5, "!"); // third slot
    pooled_unique_ptr<MyObj> p6 = pooled_unique_ptr<MyObj>(6, "!"); // forth slot - full now
    check_pooled_max_out();                                         // check that the pool max is still working
}

void test_pool_copy()
{
    std::cout << "*** test_pool_copy ***" << std::endl;
    pooled_unique_ptr<MyObj> p1(1, "hello");
    pooled_unique_ptr<MyObj> p2(2, "world");
//     p2 = p1; // should not compile
}

int main()
{
    if (MAX_POOL_SIZE != 4) {
        std::cout << "MAX_POOL_SIZE should be 4 for this simple tests" << std::endl;
        return 1;
    }

    test_pool_alloc_dealloc();
    test_pool_move();
    test_pool_move(true); // runtime error
    
//     test_pool_copy(); // compile time error

    return 0;
}