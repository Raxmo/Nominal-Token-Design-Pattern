// nominal3.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "nominal.v.3.0.h"

Datum<std::string> Name;
Behavior<void()> Print =
{
    [&]() -> void
    {
        std::cout << Print[Name] << std::endl;
    }
};

int main()
{
    std::cout << "Hello World!\n";

    Token t1;
    t1 + Name = "Tester";
    t1 += Print;

    Print();
}