#include "SystemTray.h"
#include "../../Resource/resource.h"  // 包含资源定义（如图标ID）
#include <shellapi.h>        // 包含Shell_NotifyIconW等托盘API

// 静态成员变量定义
HWND SystemTray::s_hWnd = nullptr;
HMENU SystemTray::s_hMenu = nullptr;

/**
 * @brief 创建系统托盘图标
 * @return bool 成功返回true，失败返回false
 * 
 * 该函数协调整个托盘创建流程，调用各个子功能函数
 */
bool SystemTray::CreateTray()
{
    // 创建消息窗口
    s_hWnd = CreateMessageWindow();
    if (s_hWnd == nullptr)
        return false;
    
    // 创建托盘菜单
    s_hMenu = CreateTrayMenu();
    if (s_hMenu == nullptr)
        return false;
    
    // 初始化托盘数据结构
    NOTIFYICONDATAW nid = {};
    InitializeTrayData(s_hWnd, nid);
    
    // 加载应用程序图标
    nid.hIcon = LoadApplicationIcon();
    
    // 设置托盘提示文本
    SetTrayTooltip(nid);
    
    // 将托盘图标添加到系统托盘区域
    return Shell_NotifyIconW(NIM_ADD, &nid);
}

/**
 * @brief 创建隐藏的消息窗口
 * @return HWND 成功返回窗口句柄，失败返回nullptr
 * 
 * 创建用于接收托盘消息的隐藏窗口
 */
HWND SystemTray::CreateMessageWindow()
{
    // 注册自定义窗口类
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.lpfnWndProc = TrayWndProc;              // 使用自定义窗口过程
    wcex.hInstance = GetModuleHandleW(nullptr);
    wcex.lpszClassName = L"YunsioTrayWindow";
    
    // 如果窗口类尚未注册，则注册它
    if (!GetClassInfoExW(GetModuleHandleW(nullptr), L"YunsioTrayWindow", &wcex))
    {
        if (!RegisterClassExW(&wcex))
            return nullptr;
    }
    
    // 创建隐藏的消息窗口
    HWND hWnd = CreateWindowW(
        L"YunsioTrayWindow",          // 窗口类名，使用自定义窗口类
        L"元析翻译",                  // 窗口标题（对于消息窗口无意义）
        0,                           // 窗口样式（无样式）
        0, 0, 0, 0,                 // 位置和大小（对于消息窗口无意义）
        HWND_MESSAGE,               // 父窗口句柄，HWND_MESSAGE表示创建消息窗口
        nullptr,                    // 菜单句柄（无菜单）
        GetModuleHandleW(nullptr),  // 应用程序实例句柄
        nullptr                     // 创建参数（无参数）
    );
    
    return hWnd;
}

/**
 * @brief 初始化托盘图标数据结构
 * @param hWnd 消息窗口句柄
 * @param nid 托盘图标数据结构引用
 * 
 * 设置托盘图标的基本属性和标志
 */
void SystemTray::InitializeTrayData(HWND hWnd, NOTIFYICONDATAW& nid)
{
    nid.cbSize = sizeof(NOTIFYICONDATAW);                    // 设置结构体大小
    nid.hWnd = hWnd;                                        // 设置接收消息的窗口句柄
    nid.uID = 1;                                            // 设置托盘图标的唯一标识符
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;          // 设置标志：包含图标、提示文本和消息回调
    nid.uCallbackMessage = WM_TRAYICON;                     // 设置托盘消息回调
}

/**
 * @brief 加载应用程序图标
 * @return HICON 成功返回图标句柄，失败返回nullptr
 * 
 * 尝试加载自定义图标，失败时使用系统默认图标
 */
HICON SystemTray::LoadApplicationIcon()
{
    // 首先尝试加载自定义图标资源
    HICON hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_ICON1));
    
    // 如果自定义图标加载失败，使用系统默认应用程序图标作为备选
    if (hIcon == nullptr)
    {
        hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    }
    
    return hIcon;
}

/**
 * @brief 设置托盘提示文本
 * @param nid 托盘图标数据结构引用
 * 
 * 设置鼠标悬停时显示的提示文本
 */
void SystemTray::SetTrayTooltip(NOTIFYICONDATAW& nid)
{
    // 设置托盘图标的提示文本（鼠标悬停时显示）
    wcscpy_s(nid.szTip, L"元析翻译");
}

/**
 * @brief 创建托盘右键菜单
 * @return HMENU 成功返回菜单句柄，失败返回nullptr
 * 
 * 创建包含退出选项的右键菜单
 */
HMENU SystemTray::CreateTrayMenu()
{
    // 创建弹出菜单
    HMENU hMenu = CreatePopupMenu();
    if (hMenu == nullptr)
        return nullptr;
    
    // 添加退出菜单项
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"退出");
    
    return hMenu;
}

/**
 * @brief 显示托盘右键菜单
 * @param hWnd 窗口句柄
 * 
 * 在鼠标位置显示托盘右键菜单
 */
void SystemTray::ShowTrayMenu(HWND hWnd)
{
    if (s_hMenu == nullptr)
        return;
    
    // 获取鼠标位置
    POINT pt;
    GetCursorPos(&pt);
    
    // 设置前台窗口，确保菜单能正确显示和消失
    SetForegroundWindow(hWnd);
    
    // 显示右键菜单
    TrackPopupMenu(s_hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
    
    // 发送空消息，确保菜单正确消失
    PostMessage(hWnd, WM_NULL, 0, 0);
}

/**
 * @brief 处理托盘菜单命令
 * @param commandId 菜单命令ID
 * 
 * 处理用户选择的菜单项
 */
void SystemTray::HandleMenuCommand(UINT commandId)
{
    switch (commandId)
    {
    case ID_TRAY_EXIT:
        // 退出程序
        PostQuitMessage(0);
        break;
    
    default:
        break;
    }
}

/**
 * @brief 托盘窗口消息处理过程
 * @param hWnd 窗口句柄
 * @param message 消息类型
 * @param wParam 消息参数
 * @param lParam 消息参数
 * @return LRESULT 消息处理结果
 * 
 * 处理托盘相关的窗口消息
 */
LRESULT CALLBACK SystemTray::TrayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TRAYICON:
        // 处理托盘图标消息
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONUP:
            // 右键单击，显示菜单
            ShowTrayMenu(hWnd);
            break;
        
        default:
            break;
        }
        break;
    
    case WM_COMMAND:
        // 处理菜单命令
        HandleMenuCommand(LOWORD(wParam));
        break;
    
    default:
        // 默认消息处理
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    
    return 0;
}