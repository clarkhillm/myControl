#include "WebsocketServer.h"
#include "myMesHandler.hpp"
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <thread>
#include <asio/io_service.hpp>

//The port number the WebSocket server listens on
#define PORT_NUMBER 8080

LRESULT CALLBACK WinSunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) // 通过判断消息进行消息响应
    {
        case WM_CLOSE:
            DestroyWindow(hwnd); // 销毁窗口并发送WM_DESTROY消息，但是程序没有退出
            break;
        case WM_DESTROY:
            PostQuitMessage(0); // 发出WM_QUIT消息，结束消息循环
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam); // 对不感兴趣的消息进行缺省处理，必须有该代码，否则程序有问题
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    //Create the event loop for the main thread, and the WebSocket server
    asio::io_service mainEventLoop;

    WebsocketServer server;

    //Register our network callbacks, ensuring the logic is run on the main thread's event loop
    server.connect([&mainEventLoop, &server](ClientConnection conn)
    {
        mainEventLoop.post([conn, &server]()
        {
            std::clog << "Connection opened." << std::endl;
            std::clog << "There are now " << server.numConnections() << " open connections." << std::endl;
            
            //Send a hello message to the client
            server.sendMessage(conn, "hello", Json::Value());
        });
    });
    server.disconnect([&mainEventLoop, &server](ClientConnection conn)
    {
        mainEventLoop.post([conn, &server]()
        {
            std::clog << "Connection closed." << std::endl;
            std::clog << "There are now " << server.numConnections() << " open connections." << std::endl;
        });
    });
    server.message("message", [&mainEventLoop, &server](ClientConnection conn, const Json::Value& args)
    {
        mainEventLoop.post([conn, args, &server]()
        {
            std::clog << "message handler on the main thread" << std::endl;
            std::clog << "Message payload:" << std::endl;
            for (auto key : args.getMemberNames()) {
                std::clog << "\t" << key << ": " << args[key].asString() << std::endl;
            }
            //Echo the message pack to the client
            server.sendMessage(conn, "message", args);
        });
    });
    server.message("mouse", [&mainEventLoop, &server](ClientConnection conn, const Json::Value& args)
    {
        mainEventLoop.post([conn, args, &server]()
        {
            // std::clog << "message handler on the main thread" << std::endl;
            // std::clog << "Message payload:" << std::endl;
            // for (auto key : args.getMemberNames()) {
            //     std::clog << "\t" << key << ": " << args[key].asString() << std::endl;
            // }

            mouseHandler(args["mouse"].asString(), server);
            //Echo the message pack to the client
            //server.sendMessage(conn, "mouse", args);
        });
    });

    
    //Start a keyboard input thread that reads from stdin
    // std::thread inputThread([&server, &mainEventLoop]()
    // {
    //     string input;
    //     while (1)
    //     {
    //         //Read user input from stdin
    //         std::getline(std::cin, input);
            
    //         //Broadcast the input to all connected clients (is sent on the network thread)
    //         Json::Value payload;
    //         payload["input"] = input;
    //         server.broadcastMessage("userInput", payload);
            
    //         //Debug output on the main thread
    //         mainEventLoop.post([]() {
    //             std::clog << "User input debug output on the main thread" << std::endl;
    //         });
    //     }
    // });

    

    WNDCLASS wndcls;                                            // 创建一个窗体类
    wndcls.cbClsExtra = 0;                                      // 类的额外内存，默认为0即可
    wndcls.cbWndExtra = 0;                                      // 窗口的额外内存，默认为0即可
    wndcls.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 获取画刷句柄（将返回的HGDIOBJ进行强制类型转换）
    wndcls.hCursor = LoadCursor(NULL, IDC_CROSS);               // 设置光标
    wndcls.hIcon = LoadIcon(NULL, IDI_ERROR);                   // 设置窗体左上角的图标
    wndcls.hInstance = hInst;                                   // 设置窗体所属的应用程序实例
    wndcls.lpfnWndProc = WinSunProc;                            // 设置窗体的回调函数
    wndcls.lpszClassName = _T("test");                          // 设置窗体的类名
    wndcls.lpszMenuName = NULL;                                 // 设置窗体的菜单,没有，填NULL
    wndcls.style = CS_HREDRAW | CS_VREDRAW;                     // 设置窗体风格为水平重画和垂直重画
    RegisterClass(&wndcls);                                     // 向操作系统注册窗体

    // 产生一个窗体，并返回该窗体的句柄，第一个参数必须为要创建的窗体的类名，第二个参数为窗体标题名
    HWND hwnd = CreateWindow(_T("test"), _T("test中文"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInst, NULL);

    ShowWindow(hwnd, SW_SHOWNORMAL); // 把窗体显示出来
    UpdateWindow(hwnd);              // 更新窗体

    // Start the networking thread
    std::thread serverThread([&server]() {
        server.run(PORT_NUMBER);
    });
    
    //Start the event loop for the main thread
    std::thread wsMainThread([&mainEventLoop](){
        asio::io_service::work work(mainEventLoop);
        mainEventLoop.run();
    });

    MSG msg;
    // 消息循环
    while (GetMessage(&msg, NULL, 0, 0)) // 如果消息不是WM_QUIT,返回非零值；如果消息是WM_QUIT，返回零
    {
        TranslateMessage(&msg); // 翻译消息，如把WM_KEYDOWN和WM_KEYUP翻译成一个WM_CHAR消息
        DispatchMessage(&msg);  // 派发消息
    }

    mainEventLoop.stop();
    server.getEndpoint()->stop_perpetual();
    server.getEndpoint()->stop_listening();
    server.getEndpoint()->stop();

    serverThread.join();
    wsMainThread.join();

    return 0;
}
