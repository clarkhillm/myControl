#include "myMesHandler.hpp"

#include <WinUser.h>

using namespace std;

Json::Value parseJson(const string &json)
{
    Json::Value root;
    Json::Reader reader;
    reader.parse(json, root);
    return root;
}

void keyDown(int code)
{
    keybd_event(code, 0, 0, 0);
}

void keyUp(int code)
{
    keybd_event(code, 0, KEYEVENTF_KEYUP, 0);
}

/*
 * mousemap[0][0]就表示按下鼠标左键,mousemap[0][1]就代表抬起鼠标左键
 * mousemap[1][0]表示按下鼠标滚轮键,mousemap[1][1] 表示抬起鼠标滚轮键
 * mousemap[2][0]表示按下鼠标右键,mousemap[2][1]表示抬起鼠标右键
 */
int mousemap[3][2] = {
    // 对应的值:0x0002                   0x0004
    {MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP},
    //         0x0020                   0x0040
    {MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP},
    //          0x0008                  0x0010
    {MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP}};

void mouseClick(int c) // 0:左键  1:滚轮键  2:右键
{
    mouse_event(mousemap[c][0], 0, 0, 0, 0);
    mouse_event(mousemap[c][1], 0, 0, 0, 0);
}

void mouseDown(int c)
{
    mouse_event(mousemap[c][0], 0, 0, 0, 0);
}

void mouseUp(int c)
{
    mouse_event(mousemap[c][1], 0, 0, 0, 0);
}

int mouseMoveHandler(std::string msg, WebsocketServer &server)
{
    clog << "mouse-move:\n"
         << msg << endl;
    Json::Value messageObject = parseJson(msg);

    clog << "x:" << messageObject["x"] << endl;
    clog << "y:" << messageObject["y"] << endl;

    SetCursorPos(messageObject["x"].asInt(), messageObject["y"].asInt());
    return 0;
}

int mouseDownHandler(std::string msg, WebsocketServer &server)
{
    clog << "mouse-down:\n"
         << msg << endl;
    Json::Value messageObject = parseJson(msg);
    clog << "key:" << messageObject["buttons"] << endl;

    //int key = messageObject["buttons"].asInt();

    mouseDown(messageObject["buttons"].asInt());
    // switch (key)
    // {
    // case 1:
    //     mouseDown(0);
    //     break;
    // case 2:
    //     mouseDown(2);
    //     break;
    // case 4:
    //     mouseDown(1);
    //     break;
    // }

    return 0;
}

int mouseUpHandler(std::string msg, WebsocketServer &server)
{
    clog << "mouse-up:\n"
         << msg << endl;
    Json::Value messageObject = parseJson(msg);
    clog << "key:" << messageObject["buttons"] << endl;

    mouseUp(messageObject["buttons"].asInt());
    //mouseUp(0);
    //mouseUp(1);
    //mouseUp(2);

    return 0;
}

int mouseWheelHandler(std::string msg, WebsocketServer &server)
{
    clog << "mouse-wheel:\n"
         << msg << endl;
    Json::Value messageObject = parseJson(msg);

    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, messageObject["deltaY"].asInt(), 0);
    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, messageObject["deltaX"].asInt(), 0);

    return 0;
}

int keyDownHandler(std::string msg, WebsocketServer &server)
{
    clog << "keyDown:\n"
         << msg << endl;
    Json::Value messageObject = parseJson(msg);

    clog << "key:" << messageObject["key"] << endl;
    clog << "keyCode:" << messageObject["keyCode"].asInt() << endl;

    keyDown(messageObject["keyCode"].asInt());

    return 0;
}

int keyUpHandler(std::string msg, WebsocketServer &server)
{
    clog << "keyUp:\n"
         << msg << endl;
    Json::Value messageObject = parseJson(msg);

    keyUp(messageObject["keyCode"].asInt());

    return 0;
}