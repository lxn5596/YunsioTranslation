#include "TranslationManager.h"
#include "TranslationService.h"
#ifdef _DEBUG
#include <crtdbg.h>
#endif

// 静态成员变量定义
bool TranslationManager::s_bInitialized = false;
bool TranslationManager::s_bTranslationInProgress = false;

/**
 * @brief 初始化翻译管理器
 * @return 成功返回true，失败返回false
 */
bool TranslationManager::Initialize()
{
    if (s_bInitialized)
        return true;
    
    // 初始化翻译服务
    if (!TranslationService::Initialize())
        return false;
    
    s_bInitialized = true;
    return true;
}

/**
 * @brief 清理翻译管理器资源
 */
void TranslationManager::Cleanup()
{
    if (!s_bInitialized)
        return;
    
    TranslationService::Cleanup();
    s_bInitialized = false;
}

/**
 * @brief 执行翻译流程（复制->翻译->粘贴替换）
 */
void TranslationManager::ExecuteTranslation()
{
    if (!s_bInitialized || s_bTranslationInProgress)
        return;
    
    s_bTranslationInProgress = true;
    
    // 获取当前选中的文本
    std::wstring selectedText;
    if (!GetSelectedText(selectedText) || selectedText.empty())
    {
        s_bTranslationInProgress = false;
        return;
    }
    
    // 开始翻译
    TranslationService::TranslateAsync(selectedText, OnTranslationComplete);
}

/**
 * @brief 模拟Ctrl+C复制选中文本
 * @return 成功返回true，失败返回false
 */
bool TranslationManager::CopySelectedText()
{
    INPUT inputs[4] = {};
    
    // 按下Ctrl键
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[0].ki.dwFlags = 0;
    
    // 按下C键
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'C';
    inputs[1].ki.dwFlags = 0;
    
    // 释放C键
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'C';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    
    // 释放Ctrl键
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    
    // 发送按键序列
    UINT result = SendInput(4, inputs, sizeof(INPUT));
    
    // 添加短暂延时确保按键被处理
    Sleep(50);
    
    return result == 4;
}

/**
 * @brief 直接获取当前选中的文本
 * @param text 输出获取的选中文本
 * @return 成功返回true，失败返回false
 */
bool TranslationManager::GetSelectedText(std::wstring& text)
{
    text.clear();
    
    // 备份当前剪切板内容
    std::wstring originalClipboard;
    if (!GetClipboardText(originalClipboard))
    {
        // 如果无法获取原始剪切板内容，记录空字符串
        originalClipboard.clear();
    }
    
    // 使用RAII确保剪切板内容恢复
    struct ClipboardRestorer
    {
        std::wstring original;
        ClipboardRestorer(const std::wstring& orig) : original(orig) {}
        ~ClipboardRestorer() 
        {
            // 尝试恢复原始剪切板内容，失败也不抛异常
            TranslationManager::SetClipboardText(original);
        }
    } restorer(originalClipboard);
    
    // 清空剪切板以确保获取的是新复制的内容
    if (!SetClipboardText(L""))
    {
        return false;
    }
    
    // 模拟Ctrl+C复制选中文本，增加重试机制
    bool copySuccess = false;
    for (int retry = 0; retry < 3; ++retry)
    {
        if (CopySelectedText())
        {
            copySuccess = true;
            break;
        }
        Sleep(50); // 重试前等待
    }
    
    if (!copySuccess)
    {
        return false;
    }
    
    // 等待复制完成，增加超时检查
    for (int wait = 0; wait < 10; ++wait)
    {
        Sleep(50);
        if (GetClipboardText(text) && !text.empty())
        {
            return true;
        }
    }
    
    return false;
}

/**
 * @brief 从剪切板获取文本
 * @param text 输出获取的文本
 * @return 成功返回true，失败返回false
 */
bool TranslationManager::GetClipboardText(std::wstring& text)
{
    text.clear();
    
    // 增加重试机制，处理剪切板被其他程序占用的情况
    for (int retry = 0; retry < 5; ++retry)
    {
        if (OpenClipboard(nullptr))
        {
            HANDLE hData = GetClipboardData(CF_UNICODETEXT);
            if (hData != nullptr)
            {
                wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
                if (pszText != nullptr)
                {
                    try
                    {
                        text = pszText;
                        GlobalUnlock(hData);
                        CloseClipboard();
                        return !text.empty();
                    }
                    catch (...)
                    {
                        GlobalUnlock(hData);
                        CloseClipboard();
                        return false;
                    }
                }
            }
            CloseClipboard();
            return false;
        }
        
        // 剪切板被占用，等待后重试
        Sleep(20);
    }
    
    return false;
}

/**
 * @brief 设置剪切板文本
 * @param text 要设置的文本
 * @return 成功返回true，失败返回false
 */
bool TranslationManager::SetClipboardText(const std::wstring& text)
{
    // 增加重试机制，处理剪切板被其他程序占用的情况
    for (int retry = 0; retry < 5; ++retry)
    {
        if (OpenClipboard(nullptr))
        {
            EmptyClipboard();
            
            if (text.empty())
            {
                CloseClipboard();
                return true;
            }
            
            size_t size = (text.length() + 1) * sizeof(wchar_t);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
            if (hMem == nullptr)
            {
                CloseClipboard();
                return false;
            }
            
            wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
            if (pMem != nullptr)
            {
                try
                {
                    wcscpy_s(pMem, text.length() + 1, text.c_str());
                    GlobalUnlock(hMem);
                    
                    if (SetClipboardData(CF_UNICODETEXT, hMem) != nullptr)
                    {
                        CloseClipboard();
                        return true;
                    }
                    else
                    {
                        // SetClipboardData失败，需要释放内存
                        GlobalFree(hMem);
                    }
                }
                catch (...)
                {
                    GlobalUnlock(hMem);
                    GlobalFree(hMem);
                    CloseClipboard();
                    return false;
                }
            }
            else
            {
                GlobalFree(hMem);
            }
            
            CloseClipboard();
            return false;
        }
        
        // 剪切板被占用，等待后重试
        Sleep(20);
    }
    
    return false;
}

/**
 * @brief 模拟Ctrl+V粘贴文本
 * @return 成功返回true，失败返回false
 */
bool TranslationManager::PasteText()
{
    INPUT inputs[4] = {};
    
    // 按下Ctrl键
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[0].ki.dwFlags = 0;
    
    // 按下V键
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';
    inputs[1].ki.dwFlags = 0;
    
    // 释放V键
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    
    // 释放Ctrl键
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    
    // 发送按键序列
    UINT result = SendInput(4, inputs, sizeof(INPUT));
    
    // 添加短暂延时确保按键被处理
    Sleep(50);
    
    return result == 4;
}

/**
 * @brief 翻译完成回调函数
 * @param success 翻译是否成功
 * @param result 翻译结果
 */
void TranslationManager::OnTranslationComplete(bool success, const std::wstring& result)
{
    // 使用RAII确保状态重置和资源清理
    struct StateResetter
    {
        ~StateResetter() 
        { 
            s_bTranslationInProgress = false;
            // 强制垃圾回收，清理可能的内存碎片
            #ifdef _DEBUG
            _CrtCheckMemory();
            #endif
        }
    } resetter;
    
    if (success && !result.empty())
    {
        // 备份当前剪切板内容以便后续恢复
        std::wstring originalClipboard;
        GetClipboardText(originalClipboard);
        
        // 使用RAII确保剪切板内容最终恢复
        struct ClipboardRestorer
        {
            std::wstring original;
            ClipboardRestorer(const std::wstring& orig) : original(orig) {}
            ~ClipboardRestorer() 
            {
                // 延迟恢复原始剪切板内容，确保粘贴操作完成
                Sleep(200);
                SetClipboardText(original);
            }
        } clipboardRestorer(originalClipboard);
        
        // 设置翻译结果到剪切板，增加错误处理
        if (SetClipboardText(result))
        {
            // 等待设置完成
            Sleep(50);
            
            // 粘贴翻译结果，增加重试机制
            for (int retry = 0; retry < 3; ++retry)
            {
                if (PasteText())
                {
                    break;
                }
                Sleep(50); // 重试前等待
            }
            
            // 等待粘贴完成
            Sleep(100);
        }
    }
    
    // 显式清理局部变量，释放内存
    // 注意：这里不需要手动清理，RAII会自动处理
}