#pragma once

#include <memory>

#include "declarations.h"

void ProcessResponse(const std::vector<std::byte>& data, CloseFunction close);
bool Shutdown();