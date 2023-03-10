#include "WebsocketServer.h"
#include "myMesHandler.hpp"
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <thread>
#include <asio/io_service.hpp>
#include <string>
#include<TlHelp32.h>
#include<vector>
#include<algorithm>
#include "LogUtil.hpp"

using namespace std;

// The port number the WebSocket server listens on
#define PORT_NUMBER 8080

#define IDB_ONE 3301

#define IDI_TRAY 101
#define ID_SHOW 40001
#define ID_EXIT 40002

#define WM_TRAY (WM_USER + 100)

static BOOL bListening = FALSE;

HINSTANCE hInstance;

HWND hStatic;
HWND hButton;

int clipboardChange = 0;

NOTIFYICONDATA nid; // 托盘属性
HMENU hMenu;        // 托盘菜单

struct ProcessInfo
{
    DWORD PID;
    string PName;
    ProcessInfo(DWORD PID, string PNmae) : PID(PID), PName(PNmae) {}

    bool operator<(const ProcessInfo &rhs) const
    {
        return (PID < rhs.PID);
    }
};

vector<ProcessInfo> GetProcessInfo()
{
    STARTUPINFO st;
    PROCESS_INFORMATION pi;
    PROCESSENTRY32 ps;
    HANDLE hSnapshot;
    vector<ProcessInfo> PInfo;

    ZeroMemory(&st, sizeof(STARTUPINFO));
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    st.cb = sizeof(STARTUPINFO);
    ZeroMemory(&ps, sizeof(PROCESSENTRY32));
    ps.dwSize = sizeof(PROCESSENTRY32);

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return PInfo;
    }
    if (!Process32First(hSnapshot, &ps))
    {
        return PInfo;
    }
    do
    {
        PInfo.emplace_back(ps.th32ProcessID, string(ps.szExeFile));
    } while (Process32Next(hSnapshot, &ps));
    CloseHandle(hSnapshot);
    sort(PInfo.begin(), PInfo.end());

    return PInfo;
}

void InitTray(HINSTANCE hInstance, HWND hWnd, HICON hIcon)
{
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = IDI_TRAY;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    nid.uCallbackMessage = WM_TRAY;
    nid.hIcon = hIcon;
    lstrcpy(nid.szTip, _T("VDS键盘鼠标测试"));

    hMenu = CreatePopupMenu(); // 生成托盘菜单
    // 为托盘菜单添加两个选项
    AppendMenu(hMenu, MF_STRING, ID_SHOW, TEXT("提示"));
    AppendMenu(hMenu, MF_STRING, ID_EXIT, TEXT("退出"));

    Shell_NotifyIcon(NIM_ADD, &nid);
}

std::string GetTextFromClipboard()
{
    std::string rs("");
    if (::OpenClipboard(NULL))
    {
        HGLOBAL hMem = GetClipboardData(CF_TEXT);
        if (NULL != hMem)
        {
            char *lpStr = (char *)::GlobalLock(hMem);
            if (NULL != lpStr)
            {
                log([&]()
                    {
                        std::string text("剪贴板内容:");
                        text += std::string(lpStr);
                        return text;
                    }());
                rs=std::string(lpStr);
                ::GlobalUnlock(hMem);
            }
        }
        ::CloseClipboard();
    }
    return rs;
}

LRESULT CALLBACK WinSunProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    std::string text("监听剪贴板变化 ...");
    switch (uMsg) // 通过判断消息进行消息响应
    {
    case WM_CREATE:
        bListening = AddClipboardFormatListener(hWnd);
        break;
    case WM_TRAY:
        switch (lParam)
        {
        case WM_RBUTTONDOWN:
            // 获取鼠标坐标
            POINT pt;
            GetCursorPos(&pt);
            // 解决在菜单外单击左键菜单不消失的问题
            SetForegroundWindow(hWnd);
            // 使菜单某项变灰
            // EnableMenuItem(hMenu, ID_SHOW, MF_GRAYED);
            // 显示并获取选中的菜单
            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hWnd, NULL);
            if (cmd == ID_SHOW)
                MessageBox(hWnd, _T("显示消息"), _T("消息"), MB_OK);
            if (cmd == ID_EXIT)
                PostMessage(hWnd, WM_DESTROY, NULL, NULL);
            break;
        }
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
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0); // 发出WM_QUIT消息，结束消息循环
        break;
    case WM_CLIPBOARDUPDATE:
        // MessageBox(NULL, "WM_CLIPBOARDUPDATE", "-MSG-", 0);
        clipboardChange += 1;
        text = text + std::to_string(clipboardChange);
        log([&]()
            { 
                string msg("剪贴板变化:"); 
                msg += std::to_string(clipboardChange);
                return msg;
            }());
        GetTextFromClipboard();
        // SetWindowText(hStatic, _T(text.c_str()));
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam); // 对不感兴趣的消息进行缺省处理，必须有该代码，否则程序有问题
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    log(std::string("start ..."));
    vector<ProcessInfo> PInfo = GetProcessInfo();
    log([&](){
            string logmsg("process:");
            logmsg.append(to_string(PInfo.size()));
            return logmsg;
        }());
    // Create the event loop for the main thread, and the WebSocket server
    hInstance = hInst;
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
                             NULL, NULL, hInst, NULL);

    std::string text("监听剪贴板变化 ...");
    text = text + std::to_string(clipboardChange);

    hStatic = CreateWindow(_T("Static"), _T(text.c_str()), WS_CHILD | WS_VISIBLE,
                           2, 0, 200, 30,
                           hWnd, NULL, hInst, NULL);

    hButton = CreateWindow(_T("Button"), _T("获取剪贴板内容（文本）"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                           202, 0, 200, 30,
                           hWnd, (HMENU)IDB_ONE, hInst, NULL);

    // ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    InitTray(hInst, hWnd, hIcon);

    // asio::io_service mainEventLoop;
    // WebsocketServer server;
    // registerHandler(mainEventLoop, server);
    // std::thread serverThread([&server]() { server.run(PORT_NUMBER); });
    // std::thread wsMainThread([&mainEventLoop]() { asio::io_service::work work(mainEventLoop); mainEventLoop.run(); });

    MSG msg;
    // 消息循环
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // mainEventLoop.stop();
    // server.getEndpoint()->stop_perpetual();
    // server.getEndpoint()->stop_listening();
    // server.getEndpoint()->stop();

    // serverThread.join();
    // wsMainThread.join();

    log(std::string("... exited"));

    return 0;
}