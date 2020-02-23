#pragma once
#include <istream>

enum class CachePolicy {LRU};

std::istream& operator>>(std::istream& in, CachePolicy& policy)
{
    std::string token;
    in >> token;
    if (token == "LRU")
        policy = CachePolicy::LRU;
    else 
        in.setstate(std::ios_base::failbit);
    return in;
}