#pragma once

#include <windows.h>
#include <shellapi.h>

// 托盘消息和菜单ID定义
#define WM_TRAYICON (WM_USER + 1)  // 托盘图标消息
#define ID_TRAY_EXIT 1001          // 退出菜单项ID

/**
 * @class SystemTray
 * @brief 系统托盘管理类
 * 
 * 提供静态方法来创建和管理Windows系统托盘图标
 * 该类采用静态设计，无需实例化即可使用
 */
class SystemTray
{
public:
    /**
     * @brief 创建系统托盘图标
     * @return bool 成功返回true，失败返回false
     * 
     * 该函数协调整个托盘创建流程，调用各个子功能函数
     */
    static bool CreateTray();

private:
    /**
     * @brief 创建隐藏的消息窗口
     * @return HWND 成功返回窗口句柄，失败返回nullptr
     * 
     * 创建用于接收托盘消息的隐藏窗口
     */
    static HWND CreateMessageWindow();
    
    /**
     * @brief 初始化托盘图标数据结构
     * @param hWnd 消息窗口句柄
     * @param nid 托盘图标数据结构引用
     * 
     * 设置托盘图标的基本属性和标志
     */
    static void InitializeTrayData(HWND hWnd, NOTIFYICONDATAW& nid);
    
    /**
     * @brief 加载应用程序图标
     * @return HICON 成功返回图标句柄，失败返回nullptr
     * 
     * 尝试加载自定义图标，失败时使用系统默认图标
     */
    static HICON LoadApplicationIcon();
    
    /**
     * @brief 设置托盘提示文本
     * @param nid 托盘图标数据结构引用
     * 
     * 设置鼠标悬停时显示的提示文本
     */
    static void SetTrayTooltip(NOTIFYICONDATAW& nid);
    
    /**
     * @brief 创建托盘右键菜单
     * @return HMENU 成功返回菜单句柄，失败返回nullptr
     * 
     * 创建包含退出选项的右键菜单
     */
    static HMENU CreateTrayMenu();
    
    /**
     * @brief 显示托盘右键菜单
     * @param hWnd 窗口句柄
     * 
     * 在鼠标位置显示托盘右键菜单
     */
    static void ShowTrayMenu(HWND hWnd);
    
    /**
     * @brief 处理托盘菜单命令
     * @param commandId 菜单命令ID
     * 
     * 处理用户选择的菜单项
     */
    static void HandleMenuCommand(UINT commandId);
    
    /**
     * @brief 托盘窗口消息处理过程
     * @param hWnd 窗口句柄
     * @param message 消息类型
     * @param wParam 消息参数
     * @param lParam 消息参数
     * @return LRESULT 消息处理结果
     * 
     * 处理托盘相关的窗口消息
     */
    static LRESULT CALLBACK TrayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    // 静态成员变量
    static HWND s_hWnd;     // 消息窗口句柄
    static HMENU s_hMenu;   // 托盘菜单句柄
};