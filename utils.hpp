#pragma once
#include <random>

inline int getRandomInt(int minVal, int maxVal)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(minVal, maxVal);

    return distr(gen);
}

inline void fillRandom(int minVal, int maxVal, int* arr, size_t arrSize)
{
    for (size_t i = 0; i < arrSize; i++)
    {
        arr[i] = getRandomInt(minVal, maxVal);
    }
}

inline double cosine_similarity(long *l1, long *l2, int arrSize)
{
    double dot = 0.0, denom_a = 0.0, denom_b = 0.0 ;
    
    for (int i = 0; i < arrSize; i++) {
        dot += l1[i] * l2[i] ;
        denom_a += l1[i] * l1[i] ;
        denom_b += l2[i] * l2[i] ;
    }

    return dot / (sqrt(denom_a) * sqrt(denom_b)) ;
}

inline bool areArraysEqual(long* l1, long* l2, int arrSize)
{
    bool eq = true;
    for (size_t i = 0; i < arrSize; i++)
    {
        eq &= l1[i] == l2[i];
    }

    return eq;
}