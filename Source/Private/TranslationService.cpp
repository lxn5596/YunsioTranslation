#include "TranslationService.h"
#include <winhttp.h>
#include <string>
#include <vector>
#ifdef _DEBUG
#include <crtdbg.h>
#endif

#pragma comment(lib, "winhttp.lib")

// RAII类用于自动管理WinHTTP句柄
class WinHttpHandle
{
public:
    WinHttpHandle(HINTERNET handle = nullptr) : m_handle(handle) {}
    ~WinHttpHandle() { if (m_handle) WinHttpCloseHandle(m_handle); }
    
    // 禁止拷贝
    WinHttpHandle(const WinHttpHandle&) = delete;
    WinHttpHandle& operator=(const WinHttpHandle&) = delete;
    
    // 支持移动
    WinHttpHandle(WinHttpHandle&& other) noexcept : m_handle(other.m_handle) { other.m_handle = nullptr; }
    WinHttpHandle& operator=(WinHttpHandle&& other) noexcept
    {
        if (this != &other)
        {
            if (m_handle) WinHttpCloseHandle(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }
    
    // 重置句柄
    void reset(HINTERNET handle = nullptr)
    {
        if (m_handle) WinHttpCloseHandle(m_handle);
        m_handle = handle;
    }
    
    // 获取句柄
    HINTERNET get() const { return m_handle; }
    
    // 检查是否有效
    bool valid() const { return m_handle != nullptr; }
    
    // 隐式转换为HINTERNET
    operator HINTERNET() const { return m_handle; }
    
private:
    HINTERNET m_handle;
};

// API配置常量定义

///////////////////////////////////////////////填写你的阿里百炼APIKey/////////////////////////////////////////////////////////////////
/**
 * APIKey获取地址，阿里百炼：https://bailian.console.aliyun.com
 */

const wchar_t* TranslationService::API_KEY = L"这里填写你的阿里百炼APIKey";
const char* TranslationService::SYSTEM_PROMPT = "The Following Dialogue Enters Translation Mode, Answering Questions Is Prohibited, Only The Translation Is Returned. If I Send Chinese, You Translate It Into English (Please Convert The English Translation Result To PascalCase Format, For Example: GetObject, Remove All Spaces And Special Symbols). If I Send English, You Translate It Into Chinese. If The Word Is Misspelled Or You Don't Recognize It, You Need To Judge The Probable Meaning And Translate It. Only The Translation Result Is Returned, And No Explanation Or Additional Content Is Allowed.";

// 静态成员变量定义
HINTERNET TranslationService::s_hSession = nullptr;
bool TranslationService::s_bInitialized = false;

/**
 * @brief 初始化翻译服务
 * @return 成功返回true，失败返回false
 */
bool TranslationService::Initialize()
{
    if (s_bInitialized)
        return true;
    
    // 创建HTTP会话
    s_hSession = WinHttpOpen(
        L"YunsioTranslation/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    
    if (s_hSession == nullptr)
        return false;
    
    // 设置超时时间（毫秒）
    WinHttpSetTimeouts(s_hSession, 10000, 10000, 30000, 30000);
    
    s_bInitialized = true;
    return true;
}

/**
 * @brief 清理翻译服务资源
 */
void TranslationService::Cleanup()
{
    if (!s_bInitialized)
        return;
    
    if (s_hSession != nullptr)
    {
        WinHttpCloseHandle(s_hSession);
        s_hSession = nullptr;
    }
    
    s_bInitialized = false;
}

/**
 * @brief 异步翻译文本
 * @param text 待翻译的文本
 * @param callback 翻译完成后的回调函数
 * @return 请求发送成功返回true，失败返回false
 */
bool TranslationService::TranslateAsync(const std::wstring& text, TranslationCallback callback)
{
    if (!s_bInitialized || !callback || text.empty())
        return false;
    
    // 使用RAII确保资源清理
    struct ResourceCleaner
    {
        ~ResourceCleaner()
        {
            // 强制内存清理
            #ifdef _DEBUG
            _CrtCheckMemory();
            #endif
        }
    } cleaner;
    
    try
    {
        // 连接到API服务器
        WinHttpHandle hConnect(WinHttpConnect(
            s_hSession,
            L"dashscope.aliyuncs.com",
            INTERNET_DEFAULT_HTTPS_PORT,
            0
        ));
        
        if (!hConnect.valid())
        {
            callback(false, L"连接服务器失败");
            return false;
        }
        
        // 创建请求
        WinHttpHandle hRequest(WinHttpOpenRequest(
            hConnect,
            L"POST",
            L"/compatible-mode/v1/chat/completions",
            nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE
        ));
        
        if (!hRequest.valid())
        {
            callback(false, L"创建请求失败");
            return false;
        }
    
    // 设置请求头
    std::wstring authHeader = L"Authorization: Bearer ";
    authHeader += API_KEY;
    
    WinHttpAddRequestHeaders(
        hRequest,
        L"Content-Type: application/json",
        -1,
        WINHTTP_ADDREQ_FLAG_ADD
    );
    
    WinHttpAddRequestHeaders(
        hRequest,
        authHeader.c_str(),
        -1,
        WINHTTP_ADDREQ_FLAG_ADD
    );
    
    WinHttpAddRequestHeaders(
        hRequest,
        L"User-Agent: YunsioTranslation/1.0",
        -1,
        WINHTTP_ADDREQ_FLAG_ADD
    );

        // 将待翻译文本转换为UTF-8并进行JSON转义
        std::string escapedText;
        int utf8Size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8Size > 0)
        {
            std::string utf8Text(utf8Size - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, &utf8Text[0], utf8Size, nullptr, nullptr);
            
            // 转义JSON特殊字符
            escapedText.reserve(utf8Text.length() * 2); // 预分配内存
            for (char c : utf8Text)
            {
                switch (c)
                {
                    case '"': escapedText += "\\\""; break;
                    case '\\': escapedText += "\\\\"; break;
                    case '\n': escapedText += "\\n"; break;
                    case '\r': escapedText += "\\r"; break;
                    case '\t': escapedText += "\\t"; break;
                    default: escapedText += c; break;
                }
            }
            
            // 清理utf8Text，释放内存
            utf8Text.clear();
            utf8Text.shrink_to_fit();
        }
        
        // 构建JSON请求体
        std::string jsonData;
        jsonData.reserve(1024 + strlen(SYSTEM_PROMPT) + escapedText.length()); // 预分配内存
        jsonData = "{"
            "\"model\":\"qwen-plus\","
            "\"temperature\":0.3,"
            "\"max_tokens\":1000,"
            "\"messages\":["
                "{"
                    "\"role\":\"system\","
                    "\"content\":\"" + std::string(SYSTEM_PROMPT) + "\""
                "},"
                "{"
                    "\"role\":\"user\","
                    "\"content\":\"" + escapedText + "\""
                "}"
            "]"
        "}";
        
        // 清理escapedText，释放内存
        escapedText.clear();
        escapedText.shrink_to_fit();
        
        // 发送请求
        BOOL result = WinHttpSendRequest(
            hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            (LPVOID)jsonData.c_str(),
            jsonData.length(),
            jsonData.length(),
            0
        );
        
        if (!result)
        {
            callback(false, L"发送请求失败");
            return false;
        }
        
        // 接收响应
        result = WinHttpReceiveResponse(hRequest, nullptr);
        if (!result)
        {
            callback(false, L"接收响应失败");
            return false;
        }
        
        // 清理jsonData，释放内存
        jsonData.clear();
        jsonData.shrink_to_fit();
        
        // 读取响应数据
        std::string responseData;
        responseData.reserve(4096); // 预分配内存
        DWORD bytesAvailable = 0;
        DWORD bytesRead = 0;
        
        do
        {
            if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable))
                break;
            
            if (bytesAvailable > 0)
            {
                size_t oldSize = responseData.size();
                responseData.resize(oldSize + bytesAvailable);
                if (!WinHttpReadData(hRequest, &responseData[oldSize], bytesAvailable, &bytesRead))
                    break;
                responseData.resize(oldSize + bytesRead); // 调整到实际读取的大小
            }
        } while (bytesAvailable > 0);
        
        // WinHttpHandle会自动释放句柄
        
        // 解析JSON响应
        std::wstring translatedText;
        if (ParseJsonResponse(responseData, translatedText))
        {
            callback(true, translatedText);
        }
        else
        {
            callback(false, L"解析响应失败");
        }
    
        // 显式清理大型字符串，释放内存
        responseData.clear();
        responseData.shrink_to_fit();
        
        return true;
    }
    catch (...)
    {
        // 异常处理，确保回调被调用
        callback(false, L"翻译过程中发生异常");
        return false;
    }
}

/**
 * @brief 解析JSON响应获取翻译结果
 * @param jsonResponse JSON响应字符串
 * @param result 输出翻译结果
 * @return 解析成功返回true，失败返回false
 */
bool TranslationService::ParseJsonResponse(const std::string& jsonResponse, std::wstring& result)
{
    // 简单的JSON解析，查找choices[0].message.content
    size_t choicesPos = jsonResponse.find("\"choices\"");
    if (choicesPos == std::string::npos)
        return false;
    
    size_t messagePos = jsonResponse.find("\"message\"", choicesPos);
    if (messagePos == std::string::npos)
        return false;
    
    size_t contentPos = jsonResponse.find("\"content\"", messagePos);
    if (contentPos == std::string::npos)
        return false;
    
    size_t valueStart = jsonResponse.find(':', contentPos);
    if (valueStart == std::string::npos)
        return false;
    
    valueStart = jsonResponse.find('"', valueStart);
    if (valueStart == std::string::npos)
        return false;
    
    valueStart++; // 跳过开始的引号
    
    size_t valueEnd = valueStart;
    while (valueEnd < jsonResponse.length())
    {
        if (jsonResponse[valueEnd] == '"' && (valueEnd == 0 || jsonResponse[valueEnd - 1] != '\\'))
            break;
        valueEnd++;
    }
    
    if (valueEnd >= jsonResponse.length())
        return false;
    
    std::string utf8Content = jsonResponse.substr(valueStart, valueEnd - valueStart);
    
    // 处理转义字符
    std::string unescapedContent;
    unescapedContent.reserve(utf8Content.length()); // 预分配内存
    for (size_t i = 0; i < utf8Content.length(); i++)
    {
        if (utf8Content[i] == '\\' && i + 1 < utf8Content.length())
        {
            switch (utf8Content[i + 1])
            {
                case '"': unescapedContent += '"'; i++; break;
                case '\\': unescapedContent += '\\'; i++; break;
                case 'n': unescapedContent += '\n'; i++; break;
                case 'r': unescapedContent += '\r'; i++; break;
                case 't': unescapedContent += '\t'; i++; break;
                default: unescapedContent += utf8Content[i]; break;
            }
        }
        else
        {
            unescapedContent += utf8Content[i];
        }
    }
    
    // 转换UTF-8到宽字符
    if (unescapedContent.empty())
        return false;
        
    int wideSize = MultiByteToWideChar(CP_UTF8, 0, unescapedContent.c_str(), static_cast<int>(unescapedContent.length()), nullptr, 0);
    if (wideSize > 0)
    {
        result.resize(wideSize);
        MultiByteToWideChar(CP_UTF8, 0, unescapedContent.c_str(), static_cast<int>(unescapedContent.length()), &result[0], wideSize);
        return true;
    }
    
    return false;
}