#pragma once

#include <windows.h>
#include <string>

/**
 * @class TranslationManager
 * @brief 翻译管理器 - 处理文本选择、翻译和替换的完整流程
 */
class TranslationManager
{
public:
    /**
     * @brief 初始化翻译管理器
     * @return 成功返回true，失败返回false
     */
    static bool Initialize();
    
    /**
     * @brief 清理翻译管理器资源
     */
    static void Cleanup();
    
    /**
     * @brief 执行翻译流程（复制->翻译->粘贴替换）
     */
    static void ExecuteTranslation();
    
private:
    /**
     * @brief 模拟Ctrl+C复制选中文本
     * @return 成功返回true，失败返回false
     */
    static bool CopySelectedText();
    
    /**
     * @brief 直接获取当前选中的文本
     * @param text 输出获取的选中文本
     * @return 成功返回true，失败返回false
     */
    static bool GetSelectedText(std::wstring& text);
    
    /**
     * @brief 从剪切板获取文本
     * @param text 输出获取的文本
     * @return 成功返回true，失败返回false
     */
    static bool GetClipboardText(std::wstring& text);
    
    /**
     * @brief 设置剪切板文本
     * @param text 要设置的文本
     * @return 成功返回true，失败返回false
     */
    static bool SetClipboardText(const std::wstring& text);
    
    /**
     * @brief 模拟Ctrl+V粘贴文本
     * @return 成功返回true，失败返回false
     */
    static bool PasteText();
    
    /**
     * @brief 翻译完成回调函数
     * @param success 翻译是否成功
     * @param result 翻译结果
     */
    static void OnTranslationComplete(bool success, const std::wstring& result);
    
    // 静态成员变量
    static bool s_bInitialized;
    static bool s_bTranslationInProgress;     // 翻译进行中标志
};