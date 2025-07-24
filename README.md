# YunsioTranslation - 元析翻译

一个基于通义千问API的Windows桌面翻译工具，支持全局热键快速翻译选中文本。

## 🚀 功能特性

- **全局热键翻译**: 使用 `Ctrl + 空格` 快速翻译当前选中的文本
- **智能双向翻译**: 自动识别中英文，中文翻译为英文（PascalCase格式），英文翻译为中文
- **系统托盘集成**: 最小化到系统托盘，不占用任务栏空间
- **单实例运行**: 防止重复启动，确保系统资源合理使用
- **异步翻译**: 非阻塞式翻译，不影响其他操作
- **内存优化**: 采用RAII设计模式，自动管理资源，防止内存泄漏

## 📋 系统要求

- **操作系统**: Windows 10/11
- **运行时**: Visual C++ Redistributable 2019或更高版本
- **网络**: 需要互联网连接访问通义千问API

## 🛠️ 技术架构

### 核心模块

#### 1. TranslationService (翻译服务)
- **文件**: `TranslationService.h/cpp`
- **功能**: 负责与通义千问API通信，处理HTTP请求和响应
- **特性**:
  - 使用WinHTTP库进行网络通信
  - RAII模式管理HTTP句柄
  - 支持异步翻译回调
  - 优化的JSON解析和UTF-8编码处理

#### 2. TranslationManager (翻译管理器)
- **文件**: `TranslationManager.h/cpp`
- **功能**: 协调整个翻译流程，管理剪切板操作
- **特性**:
  - 自动获取选中文本（Ctrl+C模拟）
  - 智能剪切板备份和恢复
  - 异常安全的资源管理
  - 重试机制确保操作可靠性

#### 3. GlobalHotkey (全局热键)
- **文件**: `GlobalHotkey.h/cpp`
- **功能**: 注册和处理全局热键事件
- **特性**:
  - 支持热键冲突检测和备用方案
  - 线程安全的消息处理
  - 自动清理热键注册

#### 4. SystemTray (系统托盘)
- **文件**: `SystemTray.h/cpp`
- **功能**: 管理系统托盘图标和右键菜单
- **特性**:
  - 模块化的托盘创建流程
  - 自定义图标和提示文本
  - 完整的资源清理机制

#### 5. YunsioTranslation (主程序)
- **文件**: `YunsioTranslation.h/cpp`
- **功能**: 程序入口点和主消息循环
- **特性**:
  - 单实例检测（Mutex）
  - 模块初始化和清理
  - 消息循环和事件分发

### 设计模式

- **RAII (Resource Acquisition Is Initialization)**: 自动资源管理
- **单例模式**: 确保关键服务唯一性
- **观察者模式**: 异步回调机制
- **策略模式**: 可配置的翻译策略

## 🔧 编译指南

### 开发环境
- **IDE**: Visual Studio 2019/2022
- **工具集**: MSVC v142或更高版本
- **Windows SDK**: 10.0或更高版本

### 编译步骤

1. **克隆项目**
   ```bash
   git clone <repository-url>
   cd YunsioTranslation
   ```

2. **打开解决方案**
   ```
   双击 YunsioTranslation.sln
   ```

3. **配置项目**
   - 确保目标平台为 x64
   - 选择 Release 配置（推荐）

4. **编译项目**
   ```
   生成 -> 生成解决方案 (Ctrl+Shift+B)
   ```

### 依赖库
- **WinHTTP**: Windows HTTP服务API
- **Shell32**: Windows Shell API
- **User32**: Windows用户界面API
- **Kernel32**: Windows核心API

## 📖 使用说明

### 基本使用

1. **启动程序**: 双击 `YunsioTranslation.exe`
2. **选中文本**: 在任意应用程序中选中要翻译的文本
3. **触发翻译**: 按下 `Ctrl + 空格`
4. **查看结果**: 翻译结果将自动替换选中的文本

### 翻译规则

- **中文 → 英文**: 翻译为PascalCase格式（如：GetObject）
- **英文 → 中文**: 翻译为中文释义
- **拼写错误**: 自动推断可能含义并翻译
- **仅返回翻译结果**: 不包含解释或额外内容

### 系统托盘

- **图标**: 显示在系统托盘区域
- **提示**: 鼠标悬停显示"元析翻译"
- **右键菜单**: 包含"退出"选项

## ⚙️ 配置说明

### API配置

在 `TranslationService.cpp` 中配置API参数：

API获取地址：[阿里云百炼](https://bailian.console.aliyun.com/?spm=5176.12818093_47.console-base_search-panel.dtab-product_sfm.60942cc9ZcgdUV&scm=20140722.S_sfm._.ID_sfm-RL_%E7%99%BE%E7%82%BC-LOC_console_console-OR_ser-V_4-P0_0&tab=model#/api-key)
```cpp
// API配置常量
const wchar_t* TranslationService::API_KEY = L"你的阿里百炼API密钥";
const char* TranslationService::SYSTEM_PROMPT = "翻译系统提示词";
```

### 热键配置

在 `GlobalHotkey.h` 中修改热键设置：

```cpp
#define HOTKEY_ID 1
// 默认: MOD_CONTROL + VK_SPACE (Ctrl + 空格)
```

## 🔍 故障排除

### 常见问题

1. **热键不响应**
   - 检查是否有其他程序占用了 `Ctrl + 空格`
   - 程序会自动尝试备用热键ID

2. **翻译失败**
   - 检查网络连接
   - 验证API密钥是否有效
   - 查看是否超出API调用限制

3. **程序无法启动**
   - 确保安装了Visual C++ Redistributable
   - 检查Windows版本兼容性

4. **内存占用过高**
   - 程序已优化内存使用
   - 如有问题，重启程序即可

### 调试模式

在Debug配置下编译可启用额外的内存检查：

```cpp
#ifdef _DEBUG
#include <crtdbg.h>
_CrtCheckMemory(); // 内存泄漏检测
#endif
```

## 🛡️ 安全特性

- **单实例保护**: 防止重复运行
- **异常安全**: 完整的异常处理机制
- **资源管理**: RAII确保资源正确释放
- **内存安全**: 防止缓冲区溢出和内存泄漏
- **API安全**: 安全的HTTP通信

## 📁 项目结构

```
YunsioTranslation/
├── Source/
│   ├── Public/                 # 头文件
│   │   ├── GlobalHotkey.h
│   │   ├── SystemTray.h
│   │   ├── TranslationManager.h
│   │   ├── TranslationService.h
│   │   └── YunsioTranslation.h
│   └── Private/                # 实现文件
│       ├── GlobalHotkey.cpp
│       ├── SystemTray.cpp
│       ├── TranslationManager.cpp
│       ├── TranslationService.cpp
│       └── YunsioTranslation.cpp
├── Resource/                   # 资源文件
│   ├── Translate.ico
│   ├── YunsioTranslation.rc
│   └── resource.h
├── YunsioTranslation.sln      # Visual Studio解决方案
├── YunsioTranslation.vcxproj  # 项目文件
└── README.md                  # 项目文档
```

## 🔄 版本历史

### v1.0.0
- 初始版本发布
- 基本翻译功能
- 全局热键支持
- 系统托盘集成

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 🤝 贡献指南

欢迎提交Issue和Pull Request来改进项目！

### 开发规范
- 遵循现有代码风格
- 添加适当的注释
- 确保内存安全
- 编写异常安全的代码

## 📞 联系方式

如有问题或建议，请通过以下方式联系：

- **项目Issues**: [GitHub Issues](https://github.com/lxn5596/YunsioTranslation.git/issues)
- **邮箱**: yunsio@yunsio.com

---

**YunsioTranslation** - 让翻译更简单，让编程更高效！
