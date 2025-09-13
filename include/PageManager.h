#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "WifiManager.h"
#include "weather.h"
#include "history_today.h"
#include "news.h"
#include "yiyan.h"
#include "PrefManager.h"
#include "IMAGE.h"
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>



class PageManager {
private:
    Page currentPage;
    // 记录上一次页面（用于从屏保返回）
    Page previousPage;
    
    // 页面总数（根据Page枚举最后一个元素计算）
    static const int TOTAL_PAGES = PAGE_PIC + 1;
    
public:
    PageManager();
    
    // 初始化，从主页开始
    void begin();
    
    // 获取当前页面
    Page getCurrentPage();
    
    // 处理按键事件，返回是否需要刷新屏幕的布尔值
    bool handleKeyEvent(KeyEvent event);
    
    // 进入屏保模式
    void enterScreensaver();
    
    // 从屏保模式返回
    void exitScreensaver();
    
    // 获取当前页面名称（用于调试）
    const char* getCurrentPageName();

    // 显示当前页面信息并转发到具体页面执行逻辑
    void showPage();
    // 具体页面执行逻辑
    void showHomePage(); 
    void showTodoPage(); 
    void showNewsPage(); 
    void showHistoryPage(); 
    void showPicPage(); 
};

#endif // PAGE_MANAGER_H
