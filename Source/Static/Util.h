#pragma once
#include <string>

#include "Core/Container/Array.h"

static size_t Hash(std::string Str)
{
    size_t Hash = 0;
    for (size_t i = 0; i < Str.length(); i++)
    {
        Hash = 65599 * Hash + Str[i];
    }
    return Hash ^ (Hash >> 16);
}

static TArray<std::string> Split(const std::string& str, char delim) {
    std::istringstream iss(str);
    TArray<std::string> result;
    std::string token;
    while (std::getline(iss, token, delim)) {
        if (token.empty())
            result.Add("1");
        else
            result.Add(token);
    }
    return result;
}