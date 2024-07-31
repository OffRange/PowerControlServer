#pragma once

#include <memory>

#include "declarations.h"

void ProcessResponse(const std::vector<unsigned char>& data, SendFunction send,
                     CloseFunction close);
bool Shutdown();
void RespondToHeartbeat(SendFunction send);
