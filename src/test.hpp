#include <thread>

#include <Arduino.h>

#include <shutter.hpp>

shutter::Controller *get_controller(ShutterIndex *shutter);

void test_select(ShutterIndex shutter)
{
    auto ctrl_idx = shutter / 8;
    auto controller = get_controller(&shutter);
    Serial.print("shutter: ");
    Serial.print(ctrl_idx);
    Serial.print(" | sel: ");
    Serial.println(shutter);
    controller->roll_stop(shutter);
}

void test_select_left()
{
    test_select(14);
    test_select(8);
    test_select(11);
    test_select(9);
    test_select(12);
    test_select(10);
    test_select(15);
    test_select(13);
}

void test_select_right()
{
    test_select(3);
    test_select(0);
    test_select(7);
    test_select(4);
    test_select(2);
}

void test()
{
    delay(5000);
    std::thread(test_select_left).detach();
    std::thread(test_select_right).detach();
}