/**
 * @file TranslationService.cpp
 * @brief 翻译服务类实现文件 - 使用通义千问API进行翻译
 * @author YunsioTranslation
 * @date 2024
 */

#include "TranslationService.h"
#include <winhttp.h>
#include <string>
#include <vector>

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
const wchar_t* TranslationService::API_URL = L"dashscope.aliyuncs.com";
const wchar_t* TranslationService::API_KEY = L"sk-0a685f681fee45c8891cb020c04bf1be";
const wchar_t* TranslationService::MODEL_NAME = L"qwen-plus";

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
    
    // 连接到API服务器
    WinHttpHandle hConnect(WinHttpConnect(
        s_hSession,
        API_URL,
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
    
    // 构建JSON请求体
    // 首先构建系统提示词并进行UTF-8编码
    std::wstring systemPrompt = L"接下来的对话进入翻译模式，禁止回答问题，只返回翻译。如果我发中文，你就翻译为英文（请将英文翻译结果转换为帕斯卡命名格式(PascalCase)，例如：GetObject，去除所有空格和特殊符号。），如果我发英文你就翻译为中文，如果单词拼写错误或你不认识则你需要判断大概的意思进行翻译。只需要返回翻译结果，禁止添加任何解释或额外内容。";
    
    // 转换系统提示词为UTF-8
    int systemUtf8Size = WideCharToMultiByte(CP_UTF8, 0, systemPrompt.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string systemUtf8Text;
    if (systemUtf8Size > 0)
    {
        systemUtf8Text.resize(systemUtf8Size - 1);
        WideCharToMultiByte(CP_UTF8, 0, systemPrompt.c_str(), -1, &systemUtf8Text[0], systemUtf8Size, nullptr, nullptr);
    }
    
    // 转义系统提示词中的JSON字符
    std::string escapedSystemText;
    for (char c : systemUtf8Text)
    {
        if (c == '"') escapedSystemText += "\\\"";
        else if (c == '\\') escapedSystemText += "\\\\";
        else if (c == '\n') escapedSystemText += "\\n";
        else if (c == '\r') escapedSystemText += "\\r";
        else if (c == '\t') escapedSystemText += "\\t";
        else escapedSystemText += c;
    }
    
    std::string jsonData = "{"
        "\"model\":\"qwen-plus\","
        "\"temperature\":0.3,"
        "\"max_tokens\":1000,"
        "\"messages\":["
            "{"
                "\"role\":\"system\","
                "\"content\":\"" + escapedSystemText + "\""
            "},"
            "{"
                "\"role\":\"user\","
                "\"content\":\"";
    
    // 转换文本为UTF-8
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Size > 0)
    {
        std::string utf8Text(utf8Size - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, &utf8Text[0], utf8Size, nullptr, nullptr);
        
        // 转义JSON字符串
        std::string escapedText;
        for (char c : utf8Text)
        {
            if (c == '"') escapedText += "\\\"";
            else if (c == '\\') escapedText += "\\\\";
            else if (c == '\n') escapedText += "\\n";
            else if (c == '\r') escapedText += "\\r";
            else if (c == '\t') escapedText += "\\t";
            else escapedText += c;
        }
        
        jsonData += escapedText;
    }
    
    jsonData += "\""
            "}"
        "]"
    "}";
    
    // JSON数据构建完成，准备发送请求
    
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
    
    // 读取响应数据
    std::string responseData;
    DWORD bytesAvailable = 0;
    DWORD bytesRead = 0;
    
    do
    {
        if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable))
            break;
        
        if (bytesAvailable > 0)
        {
            std::vector<char> buffer(bytesAvailable + 1);
            if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead))
            {
                buffer[bytesRead] = '\0';
                responseData += buffer.data();
            }
        }
    } while (bytesAvailable > 0);
    
    // 响应数据接收完成，开始解析
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
    
    return true;
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
    for (size_t i = 0; i < utf8Content.length(); i++)
    {
        if (utf8Content[i] == '\\' && i + 1 < utf8Content.length())
        {
            char nextChar = utf8Content[i + 1];
            if (nextChar == '"') { unescapedContent += '"'; i++; }
            else if (nextChar == '\\') { unescapedContent += '\\'; i++; }
            else if (nextChar == 'n') { unescapedContent += '\n'; i++; }
            else if (nextChar == 'r') { unescapedContent += '\r'; i++; }
            else if (nextChar == 't') { unescapedContent += '\t'; i++; }
            else unescapedContent += utf8Content[i];
        }
        else
        {
            unescapedContent += utf8Content[i];
        }
    }
    
    // 转换UTF-8到宽字符
    int wideSize = MultiByteToWideChar(CP_UTF8, 0, unescapedContent.c_str(), -1, nullptr, 0);
    if (wideSize > 0)
    {
        result.resize(wideSize - 1);
        MultiByteToWideChar(CP_UTF8, 0, unescapedContent.c_str(), -1, &result[0], wideSize);
        return true;
    }
    
    return false;
}