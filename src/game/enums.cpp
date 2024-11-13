#include "enums.hpp"

bool operator==(const BlockType& a, const int& b) { return static_cast<int>(a) == b; }
bool operator>(const BlockType& a, const int& b) { return static_cast<int>(a) > b; }
bool operator<(const BlockType& a, const int& b) { return static_cast<int>(a) < b; }
