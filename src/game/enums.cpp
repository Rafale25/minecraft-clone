#include "enums.hpp"

bool operator==(const BlockType& a, const int32_t& b) { return static_cast<int32_t>(a) == b; }
bool operator>(const BlockType& a, const int32_t& b) { return static_cast<int32_t>(a) > b; }
bool operator<(const BlockType& a, const int32_t& b) { return static_cast<int32_t>(a) < b; }
