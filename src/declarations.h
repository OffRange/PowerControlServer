#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#define DEFAULT_PORT 7848

using Port = unsigned short;

using CloseFunction = std::function<void()>;
using SendFunction  = std::function<void(std::vector<unsigned char> const&)>;

using ResponseCallback = std::function<void(std::vector<unsigned char> const&,
                                            SendFunction, CloseFunction)>;