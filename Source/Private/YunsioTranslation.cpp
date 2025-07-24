#include "YunsioTranslation.h"
#include "SystemTray.h"
#include "GlobalHotkey.h"
#include "TranslationManager.h"

// 检查是否已有实例在运行
bool YunsioTranslation::IsAlreadyRunning()
{
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"Global\\YunsioTranslation_Mutex");
    if (hMutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        if (hMutex) CloseHandle(hMutex);
        return true;
    }
    return false;
}

// 运行应用程序
int YunsioTranslation::Run()
{
    if (IsAlreadyRunning())
        return 1;
    
    // 初始化各个模块
    if (!TranslationManager::Initialize())
    {
        MessageBoxW(nullptr, L"翻译管理器初始化失败", L"错误", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    if (!GlobalHotkey::Initialize())
    {
        MessageBoxW(nullptr, L"全局热键初始化失败", L"错误", MB_OK | MB_ICONERROR);
        TranslationManager::Cleanup();
        return 1;
    }
    
    SystemTray::CreateTray();
    
    MSG msg;
    while (true)
    {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            
            // 处理热键消息
            GlobalHotkey::ProcessHotkeyMessage(&msg);
            
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        Sleep(10);
    }
    
    // 清理资源
    GlobalHotkey::Cleanup();
    TranslationManager::Cleanup();
    
    return 0;
}

// 程序入口点
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // 避免未使用参数警告
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);
    
    return YunsioTranslation::Run();
}