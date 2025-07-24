#pragma once

#include <windows.h>
#include <winhttp.h>
#include <string>
#include <functional>
#include <vector>

/**
 * @class TranslationService
 * @brief 翻译服务类 - 调用通义千问API进行文本翻译
 */
class TranslationService
{
public:
    /**
     * @brief 翻译结果回调函数类型
     * @param success 翻译是否成功
     * @param result 翻译结果文本
     */
    using TranslationCallback = std::function<void(bool success, const std::wstring& result)>;
    
    /**
     * @brief 初始化翻译服务
     * @return 成功返回true，失败返回false
     */
    static bool Initialize();
    
    /**
     * @brief 清理翻译服务资源
     */
    static void Cleanup();
    
    /**
     * @brief 异步翻译文本
     * @param text 待翻译的文本
     * @param callback 翻译完成后的回调函数
     * @return 请求发送成功返回true，失败返回false
     */
    static bool TranslateAsync(const std::wstring& text, TranslationCallback callback);
    
private:
    // API配置常量
    static const wchar_t* API_KEY;
    static const char* SYSTEM_PROMPT;
    
    /**
     * @brief 解析JSON响应获取翻译结果
     * @param jsonResponse JSON响应字符串
     * @param result 输出翻译结果
     * @return 解析成功返回true，失败返回false
     */
    static bool ParseJsonResponse(const std::string& jsonResponse, std::wstring& result);
    
    // 静态成员变量
    static HINTERNET s_hSession;
    static bool s_bInitialized;
};

// 链接WinHTTP库
#pragma comment(lib, "winhttp.lib")