#ifndef _DEBUG_H
#define _DEBUG_H

#include <iostream>
#include <vector>

#ifdef MYDB_DEBUG
// Prints anything for debug.
// e.g. DEBUG(x << y << z);
#define DEBUG(ostream_sequence)                                                \
    std::cerr << "[" << __func__ << "(), L" << __LINE__ << "] "                \
              << ostream_sequence << '\n'
#else
#define DEBUG(ostream_sequence) (void(0))
#endif

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
    os << '[';
    for (auto &e : v) {
        os << e << ',';
    }
    os << ']';
    return os;
}

#endif