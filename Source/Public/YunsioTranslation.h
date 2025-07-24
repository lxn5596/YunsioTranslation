#pragma once

#include <windows.h>

class YunsioTranslation
{
public:
    // 运行应用程序
    static int Run();
    
private:
    // 检查是否已有实例在运行
    static bool IsAlreadyRunning();
};