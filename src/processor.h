#pragma once

#include <memory>

#include "declarations.h"

void ProcessResponse(const std::vector<unsigned char>& data, SendFunction send,
                     CloseFunction close);
bool Shutdown();
bool Restart();
bool Logout();
void RespondToHeartbeat(SendFunction send);
