#include "GlobalHotkey.h"
#include "TranslationManager.h"

// 静态成员变量定义
void(*GlobalHotkey::s_HotkeyCallback)() = nullptr;
bool GlobalHotkey::s_bInitialized = false;

// 初始化全局热键监听
bool GlobalHotkey::Initialize()
{
    if (s_bInitialized)
    {
        return true;
    }
    
    // 注册Ctrl+空格热键到当前线程
    // 使用NULL作为窗口句柄，热键消息会发送到调用线程的消息队列
    if (!RegisterHotKey(NULL, HOTKEY_ID, MOD_CONTROL, VK_SPACE))
    {
        DWORD error = GetLastError();
        // 如果热键已被占用，尝试使用不同的ID
        if (error == ERROR_HOTKEY_ALREADY_REGISTERED)
        {
            // 尝试注册到ID 2
            if (!RegisterHotKey(NULL, HOTKEY_ID + 1, MOD_CONTROL, VK_SPACE))
            {
                MessageBoxW(NULL, L"热键注册失败：Ctrl+空格已被其他程序占用", L"错误", MB_OK | MB_ICONWARNING);
                return false;
            }
        }
        else
        {
            MessageBoxW(NULL, L"热键注册失败：未知错误", L"错误", MB_OK | MB_ICONERROR);
            return false;
        }
    }
    
    s_bInitialized = true;
    return true;
}

// 清理全局热键监听
void GlobalHotkey::Cleanup()
{
    if (!s_bInitialized)
    {
        return;
    }
    
    // 取消注册热键（尝试两个可能的ID）
    UnregisterHotKey(NULL, HOTKEY_ID);
    UnregisterHotKey(NULL, HOTKEY_ID + 1);
    
    s_HotkeyCallback = nullptr;
    s_bInitialized = false;
}

// 设置热键回调函数
void GlobalHotkey::SetHotkeyCallback(void(*callback)())
{
    s_HotkeyCallback = callback;
}

// 处理热键消息（需要在主消息循环中调用）
void GlobalHotkey::ProcessHotkeyMessage(MSG* msg)
{
    if (msg->message == WM_HOTKEY && (msg->wParam == HOTKEY_ID || msg->wParam == HOTKEY_ID + 1))
    {
        // 执行翻译功能
        TranslationManager::ExecuteTranslation();
        
        // 如果有回调函数，也调用它
        if (s_HotkeyCallback)
        {
            s_HotkeyCallback();
        }
    }
}