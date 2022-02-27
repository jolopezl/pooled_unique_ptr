/**
 * Compile with:
 * clang++ -std=c++17 -Wall -Wextra -g -O0 -fsanitize=address test_pooled_unique_ptr.cpp
 * g++-11 -std=c++17 -Wall -Wextra -g -O0 -fsanitize=address test_pooled_unique_ptr.cpp
 * Testes with Apple Clang 13, and GNU GCC 11.
 **/

#include "pooled_unique_ptr.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct MyObj {
    int item;
    std::string msg;
    MyObj(int t, std::string msg) : item(t), msg(msg) {}
    void greet()
    {
        std::cout << "(" << item << ") MyObj: " << msg << std::endl;
    }
};

void test_pool_alloc_dealloc()
{
    std::cout << "*** test_pool_alloc_dealloc ***" << std::endl;
    pooled_unique_ptr<MyObj> p1(1, "hello");
    pooled_unique_ptr<MyObj> p2(2, "world");
    { // let's create p3 in a different scope
        pooled_unique_ptr<MyObj> p3(3, "!");
    }
    pooled_unique_ptr<MyObj> p4(4, "!");
    pooled_unique_ptr<MyObj> p5(5, "!"); // this should still work since p3 went out of scope

    // check that the pool is working
    bool catch_exception = false;
    try {
        pooled_unique_ptr<MyObj> p5(5, "!");
    } catch (std::runtime_error& e) {
        catch_exception = true;
        std::cout << e.what() << std::endl;
    }
    assert(catch_exception && "check if the exception is thrown");
}

void test_pool_move()
{
    std::cout << "*** test_pool_move ***" << std::endl;
    pooled_unique_ptr<MyObj> p1(1, "hello");
    pooled_unique_ptr<MyObj> p2 = std::move(p1);

    p2->greet(); // should be (1) MyObj: hello
    // assert(p1 == nullptr && "p1 should be nullptr");

    pooled_unique_ptr<MyObj> p3(3, "!");
    pooled_unique_ptr<MyObj> p4;
    p4 = std::move(p3);
    p4->greet();
    assert(p4->item == 3); // should be (3) MyObj: !

    // the following compile, but will produce a runtime error
    // test with ASAN
    // p1->greet();
    // p3->greet();

    pooled_unique_ptr<MyObj> p5 = pooled_unique_ptr<MyObj>(5, "!");
    p5->greet();

    pooled_unique_ptr<MyObj> p6 = pooled_unique_ptr<MyObj>(6, "!");
    p6->greet();

    // check that the pool is working
    bool catch_exception = false;
    try {
        pooled_unique_ptr<MyObj> p5(5, "!");
    } catch (std::runtime_error& e) {
        catch_exception = true;
        std::cout << e.what() << std::endl;
    }
    assert(catch_exception && "check if the exception is thrown");
}

void test_pool_copy()
{
    std::cout << "*** test_pool_copy ***" << std::endl;
    pooled_unique_ptr<MyObj> p1(1, "hello");
    pooled_unique_ptr<MyObj> p2(2, "world");
    // p2 = p1; // should not compile
}

int main()
{
    test_pool_alloc_dealloc();
    test_pool_move();
    // test_pool_copy();
    return 0;
}