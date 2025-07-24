#pragma once

#include <windows.h>

class GlobalHotkey
{
public:
    // 初始化全局热键监听
    static bool Initialize();
    
    // 清理全局热键监听
    static void Cleanup();
    
    // 设置热键回调函数
    static void SetHotkeyCallback(void(*callback)());
    
    // 处理热键消息（需要在主消息循环中调用）
    static void ProcessHotkeyMessage(MSG* msg);
    

    
private:
    // 热键ID
    static const int HOTKEY_ID = 1;
    
    // 热键回调函数指针
    static void(*s_HotkeyCallback)();
    
    // 初始化状态
    static bool s_bInitialized;
};