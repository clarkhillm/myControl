#pragma once
#include "WebsocketServer.h"
#include <string>

int mouseHandler(std::string msg, WebsocketServer& server);
int keyboardHandler(std::string msg, WebsocketServer& server);