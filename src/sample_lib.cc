#include "sample_lib.h"

#include <iostream>

int sample_lib::HelloWorldWithValue(int value) {
    std::cout << "Hello World\n";
    return value;
}