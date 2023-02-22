#pragma once
#include "WebsocketServer.h"
#include <string>

enum SHORTCUT
{
    CTRL = VK_LCONTROL,
    SHIFT = VK_LSHIFT,
    ALT = VK_MENU,
    ESC = VK_ESCAPE,
    WIN = VK_LWIN,
    BACK = VK_BACK,
    TAB = VK_TAB,
    ENTER = VK_RETURN,
    HOME = VK_HOME,
    END = VK_END,
    DEL = VK_DELETE,
    INS = VK_INSERT,
    UP = VK_UP,
    DOWN = VK_DOWN,
    RIGHT = VK_RIGHT,
    LEFT = VK_LEFT,
    F1 = VK_F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    PRTSC = VK_PRINT,
    A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z 
};




int mouseMoveHandler(std::string msg, WebsocketServer &server);
int mouseUpHandler(std::string msg, WebsocketServer &server);
int mouseDownHandler(std::string msg, WebsocketServer &server);
int mouseWheelHandler(std::string msg, WebsocketServer &server);

int keyDownHandler(std::string msg, WebsocketServer &server);
int keyUpHandler(std::string msg, WebsocketServer &server);