#include "PageManager.h"
#include <vector>
#include <iostream>
#include <string>
// 网络
extern WifiManager wifiManager;
// 内容
extern Weather weather;
extern Yiyan yiyan;
extern HistoryToday history;
extern String history_events[HISTORY_NUM];  // 这里与子文件需要一致
extern News news;
extern PrefManager pref;
extern GxEPD2_BW<GxEPD2_583_T8, GxEPD2_583_T8::HEIGHT> display;
extern U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
extern String localIP;
extern int year, month, day, hour, minute, second;
extern const char months[13][10];
extern char weekday[10];
extern char monthStr[10];
extern int weatherid;      //根据和风天气API返回的天气ID显示对应天气
extern String todo;
extern String todos[MAXTODOCOUNT];
extern int todoCount;
extern int parseTodoTasks(const String& todo, String task[6]);
// 命名不超过8个英文字符为佳，只能有英文或数字
EpdBitmap imageArray[] = {
    {"food", food, 480, 648},
    {"flower", flower,480,480},
    {"cat", cat, 480, 648}
};
const int imageCount = sizeof(imageArray) / sizeof(imageArray[0]); // 计算图片总数

char hourAndMinute[9] = ""; 
char date_y_m_d[20] = "";
uint16_t wifiIcon = 0x011b; // 默认wifi不连接，是一个叉的标志




PageManager::PageManager() {
    currentPage = PAGE_HOME;
    previousPage = PAGE_HOME;
}

void PageManager::begin() {
    currentPage = PAGE_HOME;
    previousPage = PAGE_HOME;
}

Page PageManager::getCurrentPage() {
    return currentPage;
}

bool PageManager::handleKeyEvent(KeyEvent event) {
    bool needRefresh = false;
    
    switch(event) {
        case KEY_NEXT_CLICK:
            // 单击Next切换到下一页面
            if (currentPage == PAGE_PIC) {
                // 从屏保直接切换到主页
                previousPage = currentPage;
                currentPage = PAGE_HOME;
            } else {
                // 循环切换页面
                previousPage = currentPage;
                currentPage = (Page)((currentPage + 1) % TOTAL_PAGES);
            }
            needRefresh = true;
            break;
            
        case KEY_HOME_CLICK:
            // 单击Home返回主页（如果当前已经是主页则不处理）
            // if (currentPage != PAGE_HOME) {
            //     previousPage = currentPage;
            //     currentPage = PAGE_HOME;
            //     needRefresh = true;
            // }
            if (currentPage != PAGE_PIC) {
                previousPage = currentPage;
                currentPage = PAGE_PIC;
                needRefresh = true;
            }
            break;
            
        case KEY_NEXT_LONG:
            // 长按Next刷新本页
            needRefresh = true;
            break;
            
        case KEY_HOME_LONG:
            // 长按Home重启
            // Serial.println("重启中...(仅调试，实际没有重启)");
            // needRefresh = true;
            // ESP.restart();
            break;
            
        default:
            // 无事件，不需要刷新
            needRefresh = false;
            break;
    }
    
    return needRefresh;
}

void PageManager::enterScreensaver() {
    if (currentPage != PAGE_PIC) {
        previousPage = currentPage;
        currentPage = PAGE_PIC;
    }
}

void PageManager::exitScreensaver() {
    if (currentPage == PAGE_PIC) {
        currentPage = previousPage;
    }
}

const char* PageManager::getCurrentPageName() {
    switch(currentPage) {
        case PAGE_HOME: return "Home page";
        case PAGE_TODO: return "Todo page";
        case PAGE_NEWS: return "News page";
        case PAGE_HISTORY: return "History page";
        case PAGE_PIC: return "Picture page";
        default: return "Unknown";
    }
}

void PageManager::showPage() {
    Serial.print("转发到具体页面...");
    switch(currentPage) {
        case PAGE_HOME: showHomePage(); break;
        case PAGE_TODO: showTodoPage(); break;
        case PAGE_NEWS: showNewsPage(); break;
        case PAGE_HISTORY: showHistoryPage(); break;
        case PAGE_PIC: showPicPage(); break;
        default: Serial.println("无法转发到任何页面"); break;
    }
    Serial.println("showpage done");
    // display.powerOff();
    // Serial.println("当前页面显示任务完成，屏幕和ESP32休眠..."); //写在这里甚至打印不完就休眠了，所以要加一个延时
    // delay(500);
    // esp_light_sleep_start();
    
    // print_wakeup_reason();
    // Serial.println("轻度睡眠被唤醒!");
    // display.init();

    
}
// u8g2_font_wqy16_t_gb2312
// u8g2_font_fub49_tn 大型时分秒
// u8g2_font_ncenB24_tf 日期 可做标题的大字
// u8g2_font_crox4tb_tr  yiyan
// u8g2_font_wqy12_t_gb2312 中文小字
// u8g2_font_open_iconic_all_2x_t 小图标
// u8g2_font_open_iconic_weather_2x_t 天气小图标
// u8g2_font_open_iconic_all_4x_t 包含网络标志的大图标

void PageManager::showHomePage()
{
    // 本页相关内容：日期、星期、日期号数、月份、连接状态、一言、天气图标、天气、天气地点、气温
    int16_t tbx, tby; uint16_t tbw, tbh;
    String s = yiyan.getSentence();
    localIP = wifiManager.getIP();

    Serial.println(getCurrentPageName());
    display.setFullWindow();
    sprintf(date_y_m_d,"%04d-%02d-%02d",year,month,day);
    display.firstPage();
    do{
        //1mm->5.3pixel
        // 左上角日期/星期
        display.fillScreen(GxEPD_WHITE);
        u8g2Fonts.setCursor(LEFTLINE, TOPLINE);
        u8g2Fonts.setFont(u8g2_font_ncenB12_te);
        u8g2Fonts.print(date_y_m_d);

        display.getTextBounds(date_y_m_d,u8g2Fonts.getCursorX(),u8g2Fonts.getCursorY(),&tbx,&tby,&tbw,&tbh);
        //Serial.printf("%s, %d, %d, %d, %d\n",date_y_m_d, tbx, tby, tbw, tbh);
        u8g2Fonts.setCursor(LEFTLINE, TOPLINE + tbh * 2 + V_LINESPACE);
        u8g2Fonts.print(weekday);
        // 右上角ip地址
        display.getTextBounds(localIP,u8g2Fonts.getCursorX(),u8g2Fonts.getCursorY(),&tbx,&tby,&tbw,&tbh);
        //Serial.printf("%s, %d, %d, %d, %d\n",localIP, tbx, tby, tbw, tbh);
        u8g2Fonts.setCursor(RIGHTLINE - tbw - 50, TOPLINE);
        //Serial.printf("在主页显示IP: %s\n",localIP.c_str());
        u8g2Fonts.print(localIP);
        u8g2Fonts.setFont(u8g2_font_open_iconic_all_2x_t);
        if (wifiManager.isConnected()) {wifiIcon = 0x00f8;} // 如果连接上了wifi，就是个wifi图标
        else wifiIcon = 0x011b;
        u8g2Fonts.drawGlyph(RIGHTLINE - tbw - 70, TOPLINE, wifiIcon);
        // 中间 日期和月份
        u8g2Fonts.setFont(u8g2_font_logisoso54_tn);
        display.getTextBounds(String(day),u8g2Fonts.getCursorX(),u8g2Fonts.getCursorY(),&tbx,&tby,&tbw,&tbh);
        //Serial.printf("%s, %d, %d, %d, %d\n",String(day), tbx, tby, tbw, tbh);
        u8g2Fonts.setCursor(V_HALFLINE - 30, H_HALFLINE - 50);
        u8g2Fonts.print(day);

        u8g2Fonts.setFont(u8g2_font_ncenB12_te);
        display.getTextBounds(monthStr,u8g2Fonts.getCursorX(),u8g2Fonts.getCursorY(),&tbx,&tby,&tbw,&tbh);
        //Serial.printf("月份%s, %d, %d, %d, %d\n", monthStr, tbx, tby, tbw, tbh);
        u8g2Fonts.setCursor(V_HALFLINE - tbw / 2 - 20, H_HALFLINE);
        u8g2Fonts.print(monthStr);

        // 中间 一言
        u8g2Fonts.setFont( u8g2_font_crox4tb_tr);
        display.getTextBounds(s,u8g2Fonts.getCursorX(),u8g2Fonts.getCursorY(),&tbx,&tby,&tbw,&tbh);
        // Serial.printf("一言%s, %d, %d, %d, %d\n", s.c_str(), tbx, tby, tbw, tbh);
        u8g2Fonts.setCursor(V_HALFLINE - (s.length() * 8 ) / 2, H_HALFLINE + H_1OVER4);
        // u8g2Fonts.print(s);
        DrawMultiLineString(s, 50, H_HALFLINE + H_1OVER4, display.width() - 2 * 50, 25, true);
        // 下方 天气
        if (weather.IconId != -1){
        //Serial.printf("weather IconId: %d\n",weather.IconId);
        u8g2Fonts.setFont(u8g2_font_open_iconic_weather_2x_t);
        u8g2Fonts.drawGlyph(LEFTLINE ,BOTTOMLINE,weather.IconId); // u8g2_font_open_iconic_weather_4x_t 天气图标
        }
        u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
        u8g2Fonts.setCursor(V_HALFLINE - V_1OVER4 / 2 + 10, BOTTOMLINE);
        u8g2Fonts.printf("%s : %s",weather.getLocaName().c_str(), weather.todayWeaIcon);
        u8g2Fonts.setCursor(RIGHTLINE - 65, BOTTOMLINE);
        u8g2Fonts.printf("%d - %d 度", weather.todayTempMin, weather.todayTempMax);
    }
    while (display.nextPage());
}

void PageManager::showTodoPage()
{
    Serial.println(getCurrentPageName());
    pref.load();
    todo = pref.getTodo();
    todoCount = parseTodoTasks(todo, todos);
    //Serial.printf("todo: %s\n",todo.c_str());
    uint16_t startx = TITLEX, boxh = BOXHEIGHT, boxw = display.width()-2*startx;
    display.firstPage();
    do{
    display.fillScreen(GxEPD_WHITE);
    // TODO页标题
    u8g2Fonts.setFont(u8g2_font_open_iconic_all_4x_t);
    u8g2Fonts.drawGlyph(TITLEX - 20,TITLEY,264);// TODO图样-
    u8g2Fonts.setFont(u8g2_font_ncenB24_tf);
    u8g2Fonts.setCursor(TITLEX + 30, TITLEY);
    u8g2Fonts.print("TODO");
    for (int j = 0; j< todoCount;j++){
        u8g2Fonts.setFont(u8g2_font_open_iconic_embedded_2x_t);
        u8g2Fonts.drawGlyph(startx-20,(j+1)*boxh+TITLE_CONTENT_VSPACE,69);
        u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312);
        u8g2Fonts.setCursor(startx,(j+1)*boxh+TITLE_CONTENT_VSPACE);
        u8g2Fonts.print(todos[j]);
    }
    }while (display.nextPage());
}

void PageManager::showNewsPage()
{
    Serial.println(getCurrentPageName());
    String newsList[NEWSNUM];
    uint16_t startx = TITLEX, boxh = BOXHEIGHT, boxw = display.width()-2*startx;
    display.firstPage();
    do{
    display.fillScreen(GxEPD_WHITE);
    // news页标题
    u8g2Fonts.setFont(u8g2_font_open_iconic_all_4x_t);
    u8g2Fonts.drawGlyph(TITLEX - 20,TITLEY,0x00af);// news图样-地球
    u8g2Fonts.setFont(u8g2_font_ncenB24_tf);
    u8g2Fonts.setCursor(TITLEX + 30, TITLEY);
    u8g2Fonts.print("NEWS");
    for (int j = 0; j< NEWSNUM;j++){
        u8g2Fonts.setFont(u8g2_font_open_iconic_embedded_2x_t);
        u8g2Fonts.drawGlyph(startx-20,(j+1)*boxh+TITLE_CONTENT_VSPACE,69);
        newsList[j] = news.getNews();
        u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312);
        DrawMultiLineString(newsList[j],startx,(j+1)*boxh+TITLE_CONTENT_VSPACE,boxw,u8g2Fonts.getFontAscent()-u8g2Fonts.getFontDescent()+V_LINESPACE*2,j==NEWSNUM-1); // j==NEWSNUM-1表示这次调用DrawMultiLineString是这一组输出中的最后一条新闻，可以一并输出了
    }
    }while (display.nextPage());
    
}

void PageManager::showHistoryPage()
{
Serial.println(getCurrentPageName());
    String newsList[NEWSNUM];
    uint16_t startx = TITLEX, boxh = BOXHEIGHT - 20 , boxw = display.width()-2*startx;
    display.firstPage();
    do{
    display.fillScreen(GxEPD_WHITE);
    // history页标题
    u8g2Fonts.setFont(u8g2_font_open_iconic_all_4x_t);
    u8g2Fonts.drawGlyph(TITLEX - 20,TITLEY,0x0061);// history图样-书本
    u8g2Fonts.setFont(u8g2_font_ncenB24_tf);
    u8g2Fonts.setCursor(TITLEX + 30, TITLEY);
    u8g2Fonts.print("HISTORY");
    for (int j = 0; j< HISTORY_NUM;j++){
        u8g2Fonts.setFont(u8g2_font_open_iconic_embedded_2x_t);
        u8g2Fonts.drawGlyph(startx-20,(j+1)*boxh+TITLE_CONTENT_VSPACE,69);
        history_events[j] = history.getEvent(j);
        u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312);
        DrawMultiLineString(history_events[j],startx,(j+1)*boxh+TITLE_CONTENT_VSPACE,boxw,u8g2Fonts.getFontAscent()-u8g2Fonts.getFontDescent()+V_LINESPACE*2,j==HISTORY_NUM-1);
    }
    }while (display.nextPage());
}

void PageManager::showPicPage()
{
    static int picIndex = 0;
    Serial.println(getCurrentPageName());
    //Serial.printf("图片总数: %d pictoshow: %s \n", imageCount,imageArray[picIndex].name);
    display.fillScreen(GxEPD_WHITE);
    int16_t picWidth = imageArray[picIndex].width;
    int16_t picHeight = imageArray[picIndex].height;
    int16_t startX = (display.width() - picWidth) / 2;
    int16_t startY = (display.height() - picHeight) / 2;
    // 居中显示图片
    display.drawInvertedBitmap(startX, startY, imageArray[picIndex].data,picWidth,picHeight,GxEPD_BLACK);
    display.display();
    Serial.println("display fig done");
    picIndex = (picIndex + 1) % imageCount;
}
