#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#define DEFAULT_PORT 7848

using Port = unsigned short;

using CloseFunction = std::function<void()>;

using ResponseCallback =
    std::function<void(std::vector<std::byte> const&, CloseFunction)>;