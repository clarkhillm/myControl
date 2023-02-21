#include "WebsocketServer.h"
#include "myMesHandler.hpp"
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <thread>
#include <asio/io_service.hpp>
#include <string>

//The port number the WebSocket server listens on
#define PORT_NUMBER 8080

#define IDB_ONE     3301 

static BOOL bListening = FALSE;

HINSTANCE hInstance;

HWND hStatic;
HWND hButton;

int clipboardChange = 0;

BOOL GetTextFromClipboard()
{
    if(::OpenClipboard(NULL))
    {
        HGLOBAL hMem = GetClipboardData(CF_TEXT);
        if(NULL != hMem)
        {
            char* lpStr = (char*)::GlobalLock(hMem); 
            if(NULL != lpStr)
            {
                printf("%s",lpStr);
                ::GlobalUnlock(hMem);
            }
        }
        ::CloseClipboard();
        return TRUE;
    }
    return FALSE;
}

LRESULT CALLBACK WinSunProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    std::string text("监听剪贴板变化 ...");
    switch (uMsg) // 通过判断消息进行消息响应
    {
        case WM_CREATE:
            bListening = AddClipboardFormatListener(hWnd);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDB_ONE:
                    MessageBox(hWnd, _T("您点击了第一个按钮。"), _T("提示"), MB_OK | MB_ICONINFORMATION);
                break;
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hWnd); // 销毁窗口并发送WM_DESTROY消息，但是程序没有退出
            break;
        case WM_DESTROY:
            PostQuitMessage(0); // 发出WM_QUIT消息，结束消息循环
            break;
        case WM_CLIPBOARDUPDATE:
            //MessageBox(NULL, "WM_CLIPBOARDUPDATE", "-MSG-", 0);
            clipboardChange += 1;
            text = text + std::to_string(clipboardChange);
            SetWindowText(hStatic, _T(text.c_str()));

            break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam); // 对不感兴趣的消息进行缺省处理，必须有该代码，否则程序有问题
    }
    return 0;
}

int registerHandler(asio::io_service& mainEventLoop,WebsocketServer& server)
{
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
            mousePositionHandler(args["position"].asString(), server);
            mouseButtonHandler(args["button"].asString(), server);
        });
    });
    server.message("keyboard", [&mainEventLoop, &server](ClientConnection conn, const Json::Value& args)
    {
        mainEventLoop.post([conn, args, &server]()
        {
            keyboardHandler(args["keyboard"].asString(), server);
        });
    });

    return 0;
}

int main(int argc, char* argv[])
{
    //Create the event loop for the main thread, and the WebSocket server
    asio::io_service mainEventLoop;
    WebsocketServer server;
    registerHandler(mainEventLoop,server);
    //Start the networking thread
    std::thread serverThread([&server]() {
        server.run(PORT_NUMBER);
    });
    //Start the event loop for the main thread
    asio::io_service::work work(mainEventLoop);
    mainEventLoop.run();
    return 0;
}

/*
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    //Create the event loop for the main thread, and the WebSocket server
    hInstance = hInst;
    asio::io_service mainEventLoop;

    WebsocketServer server;

    registerHandler(mainEventLoop,server);


    HICON hIcon = (HICON)::LoadImage(hInst, _T("C:\\Users\\gavin\\work\\myControl\\res\\daicy.ico"), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

    WNDCLASS wndcls;                                            // 创建一个窗体类
    wndcls.cbClsExtra = 0;                                      // 类的额外内存，默认为0即可
    wndcls.cbWndExtra = 0;                                      // 窗口的额外内存，默认为0即可
    wndcls.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 获取画刷句柄（将返回的HGDIOBJ进行强制类型转换）
    wndcls.hCursor = LoadCursor(NULL, IDC_CROSS);               // 设置光标
    wndcls.hIcon = hIcon;                                       // 设置窗体左上角的图标
    wndcls.hInstance = hInst;                                   // 设置窗体所属的应用程序实例
    wndcls.lpfnWndProc = WinSunProc;                            // 设置窗体的回调函数
    wndcls.lpszClassName = _T("test");                          // 设置窗体的类名
    wndcls.lpszMenuName = NULL;                                 // 设置窗体的菜单,没有，填NULL
    wndcls.style = CS_HREDRAW | CS_VREDRAW;                     // 设置窗体风格为水平重画和垂直重画
    RegisterClass(&wndcls);                                     // 向操作系统注册窗体


    int height = 300;
    int width = 500;

    // Create the main window
    HWND hWnd = CreateWindow(_T("test"), _T("VDS键盘鼠标测试"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        width, height, 
        NULL, NULL, hInst, NULL
    );

    std::string text("监听剪贴板变化 ...");
    text = text + std::to_string(clipboardChange);

    hStatic = CreateWindow(_T("Static"), _T(text.c_str()), WS_CHILD | WS_VISIBLE,
                           2, 0, 200, 30,
                           hWnd, NULL, hInst, NULL);

    hButton = CreateWindow(_T("Button"), _T("获取剪贴板内容（文本）"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                           202, 0, 200, 30,
                           hWnd, (HMENU)IDB_ONE, hInst, NULL);

    ShowWindow(hWnd, SW_SHOWNORMAL); // 把窗体显示出来
    UpdateWindow(hWnd);              // 更新窗体

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
        TranslateMessage(&msg); // 翻译消息，如把WM_KEYDOWN和WM_KEYUP翻译成一个WM_CHAR消f
        DispatchMessage(&msg);  // 派发消息
    }

    mainEventLoop.stop();
    server.getEndpoint()->stop_perpetual();
    server.getEndpoint()->stop_listening();
    server.getEndpoint()->stop();

    serverThread.join();
    wsMainThread.join();

    return 0;
}*/
