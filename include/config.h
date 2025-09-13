#ifndef CONFIG_H
#define CONFIG_H
#define HISTORY_NUM 9  // 存储历史事件的数量
#define YIYANNUM 5
#define NEWSNUM 6 // 存储新闻的条数
#define TOUCH_NEXT 4
#define TOUCH_HOME 32
#define USE_HSPI_FOR_EPD
#define BOTTOMLINE 630
#define TOPLINE 30
#define LEFTLINE 10
#define RIGHTLINE 470
#define H_HALFLINE 648 / 2
#define V_HALFLINE 480 / 2
#define V_LINESPACE 5
#define H_1OVER4 (H_HALFLINE / 2)
#define V_1OVER4 (V_HALFLINE / 2)
#define TITLEX 50
#define TITLEY 55
#define MAXTODOCOUNT 6
#define BOXHEIGHT 80
#define TITLE_CONTENT_VSPACE 50
#include <Arduino.h>

// 定义按键事件
enum ButtonEvent {
    BUTTON_NONE = 0,
    BUTTON_SINGLE,
    BUTTON_LONG
};
// 定义页面枚举 注意换页逻辑中假定PIC页是最后一页
enum Page {
    PAGE_HOME,        // 主页
    PAGE_TODO,        // 代办页
    PAGE_NEWS,        // 新闻页
    PAGE_HISTORY,     // 历史上的今天页
    PAGE_PIC  // 屏保页
};

// 定义按键名-事件绑定 枚举
enum KeyEvent {
    KEY_EVENT_NONE,       // 无事件
    KEY_NEXT_CLICK,       // Next单击
    KEY_NEXT_LONG,  // Next长按
    KEY_HOME_CLICK,       // Home单击
    KEY_HOME_LONG   // Home长按
};

void DrawMultiLineString(String content, uint16_t x, uint16_t y,uint16_t contentAreaWidthWithMargin, uint16_t lineHeight, bool final = false);

#endif