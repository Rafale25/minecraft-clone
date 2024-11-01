#pragma once

#include <sstream>
#include <iostream>

// usage:   SS("xyz" << 123 << 45.6) returning a std::string rvalue.
#define SS(x) ( ((std::stringstream&)(std::stringstream() << x )).str())
#define SC(x) ( ((std::stringstream&)(std::stringstream() << x )).str().c_str())
