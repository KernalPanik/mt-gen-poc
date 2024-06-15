#include "pow.hpp"

#include <stdexcept>

long simplePow(int x, int e)
{
    if (e < 0) {
        throw std::invalid_argument("simplePow: e cannot be less than 0!");
    }

    long result = 1;
    long k2p = x;

    while (e != 0) {
        if ((e & 0x1) != 0) {
            result *= k2p;
        }
        k2p *= k2p;
        e >>= 1;
    }

    return result;
}

long simplePowMutant(int x, int e)
{
    if (e < 0) {
        throw std::invalid_argument("simplePow: e cannot be less than 0!");
    }

    long result = 1;
    long k2p = x;

    while (e != 0) {
        if ((e & 0x1) != 0) {
            result *= k2p;
        }
        e >>= 1;
    }

    return result;
}