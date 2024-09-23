#include "debug.h"
#include <iostream>

int main() {
    std::cout << "Hello World!\n";
    int x = 3;
    DEBUG("x: " << x);
    return 0;
}