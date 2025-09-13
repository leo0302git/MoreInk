#include "config.h"
#include "PageManager.h"
#include "touch.h"
#include "WifiManager.h"
#include "weather.h"
#include "history_today.h"
#include "news.h"
#include "yiyan.h"
#include "PrefManager.h"
#include "IMAGE.h"
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <esp_sleep.h>
// 按键
TouchButton btnNext(TOUCH_NEXT, 50, 1000, 10, 200, 60, 60, 0);
TouchButton btnHome(TOUCH_HOME, 50, 1000, 10, 200, 60, 60, 9);
// 按键-事件变量
KeyEvent keyEvent = KEY_EVENT_NONE;
// 网络
WifiManager wifiManager;
// 内容
Weather weather;
Yiyan yiyan;
HistoryToday history;
String history_events[HISTORY_NUM];  // 这里与子文件需要一致
News news;
PrefManager pref;
String localIP;
// 日期与时间
char timeStr[64]; // 提供一个缓冲区存储时间
int year, month, day, hour, minute, second;
char monthStr[10];
const char months[13][10] = {
        "",         // 占位：索引 0 不用
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"
    };
char weekday[10];

GxEPD2_BW<GxEPD2_583_T8, GxEPD2_583_T8::HEIGHT> display(GxEPD2_583_T8(/*CS=5*/ 27, /*DC=*/ 14, /*RST=*/ 12, /*BUSY=*/ 13)); // GDEW0583T8 648x480, EK79655 (GD7965)
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
// 页面切换
PageManager page;
// 待办事项
String todo;
String todos[MAXTODOCOUNT];
int todoCount = -1;
EpdBitmap initialPages[] = {
    {"logo",logo,480, 648}
};

void DrawMultiLineString(String content, uint16_t x, uint16_t y,uint16_t contentAreaWidthWithMargin, uint16_t lineHeight, bool final)
{
    String currentLine = "";  // 当前行内容
    size_t i = 0;

    while (i < content.length())
    {
        // 检测当前 UTF-8 字符的字节数
        unsigned char byte = (unsigned char)content[i];
        size_t len;
        if (byte >= 0xFC)      len = 6;
        else if (byte >= 0xF8) len = 5;
        else if (byte >= 0xF0) len = 4;
        else if (byte >= 0xE0) len = 3;
        else if (byte >= 0xC0) len = 2;
        else                   len = 1;

        // 截取当前字符
        String ch = content.substring(i, i + len);

        // 先试着计算加上这个字符后的宽度
        String testLine = currentLine + ch;
        int16_t wTemp = u8g2Fonts.getUTF8Width(testLine.c_str());

        if (wTemp > contentAreaWidthWithMargin) {
            // 先绘制当前行
            //Serial.println(currentLine); // 调试输出
            u8g2Fonts.setCursor(x, y);
            u8g2Fonts.print(currentLine);
            //display.display(true); // 若加上这行，就是每写一行，局刷一次，太慢

            y += lineHeight;       // 换行
            currentLine = ch;      // 把这个字符放到新行
        } else {
            currentLine = testLine; // 正常加入
        }

        i += len; // 移动到下一个字符
    }

    // 绘制最后一行（如果有）
    if (currentLine.length() > 0) {
        //Serial.println(currentLine);
        u8g2Fonts.setCursor(x, y);
        u8g2Fonts.print(currentLine);
        if (final) display.display(true); // final用来判断本次对本函数的调用是否是这一组显示的最后一次，如果是，才输出缓冲区，如果不是，就不输出。这样可以将多条多行输出一次性局刷完。
    }
}

void onBootShow(){
  display.fillScreen(GxEPD_WHITE);
  delay(500);
  display.drawInvertedBitmap(0, 0, initialPages[0].data,initialPages[0].width,initialPages[0].height,GxEPD_BLACK);
  display.display();
  display.fillScreen(GxEPD_WHITE);
  delay(2000);
  // @2025 Leo. All rights reserved.
}

void networkStateShow(){
  String networkInfoCN;
  String networkInfoEN;
  uint16_t delayTime;
  if (wifiManager.isConnected()){
    networkInfoCN = "网络连接成功！IP地址见主页右上角。正在获取天气信息等，请稍候。";
    networkInfoEN = "Network connection successful! See IP address in the upper right corner of the homepage. Retrieving weather information and other stuffs. Please wait a moment.";
    delayTime = 1000;
  }
  else{
    networkInfoCN = "网络连接失败！请用其他设备连接wifi: MoreInk-ESP32, 密码: 11223333。在浏览器中访问主页右上角的IP地址, 并填入可用的wifi名称与密码。重启以重新联网。";
    networkInfoEN = "Network connection failed! Please use another device to connect to the WiFi: MoreInk-ESP32, password: 11223333. Access the IP address in the upper right corner of the homepage in the browser and enter the available WiFi name and password. Restart to reconnect to the network.";
    delayTime = 5000;
  }
  DrawMultiLineString(networkInfoCN,TITLEX, H_HALFLINE,display.width()-2*TITLEX,u8g2Fonts.getFontAscent()-u8g2Fonts.getFontDescent()+V_LINESPACE*2,false);
  DrawMultiLineString(networkInfoEN,TITLEX, H_HALFLINE + 80,display.width()-2*TITLEX,u8g2Fonts.getFontAscent()-u8g2Fonts.getFontDescent()+V_LINESPACE*2,true);
  display.display();  // 执行刷新使设置生效

  delay(delayTime);
}

int parseTodoTasks(const String& todo, String task[6]) {
    int taskCount = 0;
    String normalized = todo;

    // 将所有中文分号替换为英文分号，统一处理
    normalized.replace("；", ";");

    // 按分号分割
    int start = 0;
    while (start < normalized.length() && taskCount < 6) {
        int idx = normalized.indexOf(';', start);

        String item;
        if (idx == -1) { 
            // 最后一段
            item = normalized.substring(start);
            start = normalized.length();
        } else {
            item = normalized.substring(start, idx);
            start = idx + 1;
        }

        // 去除前后空格
        item.trim();

        // 跳过空任务
        if (item.length() > 0) {
            task[taskCount++] = item;
        }
    }

    return taskCount;
}

void updateInfo(){
  static int tryReconnectWifiCount = 1;
  if (!wifiManager.isConnected()){
    if (tryReconnectWifiCount <= 3 ){
      Serial.printf("Trying to reconnect wifi: %d times.\n", tryReconnectWifiCount);
      wifiManager.tryConnectSTA();
      tryReconnectWifiCount ++;
    }
    else{
      wifiManager.loop();
    }

  }
  wifiManager.parseLocalTime(timeStr, year, month, day, hour, minute);
  localIP = wifiManager.getIP();
  if (pref.changed) pref.load();
  if (wifiManager.isConnected() && tryReconnectWifiCount > 1){
    tryReconnectWifiCount = 1;
    wifiManager.getNetworkTime(timeStr, sizeof(timeStr));
    Serial.println(timeStr);
    wifiManager.parseTimeString(timeStr, year, month, day, hour, minute, second, weekday, monthStr);
    if (!weather.updated){
      Serial.println("Updating weather...");
      String apiKey = pref.getApiKey();
      String location = pref.getLocationID();
      String locationName = pref.getLocationName();
      weather.reConfig(apiKey,location,locationName);
      bool isDaytime = true; //默认白天 只有在开机的时候才判断是不是白天
      if (hour > 18){isDaytime = false;}
      weather.update();
      weather.setParams(true,isDaytime); // 暂时都是获取今天白天数据
      weather.showWeather();
    }
    if (!(history.isupdated)) {
      Serial.println("Updating history...");
      history.updateHistory();  // 获取历史数据
    } 
    if (!(news.isUpdated)){
      Serial.println("Updating news...");
      news.updateNews();
      news.printAllNews();
    }
    if (!yiyan.isOnline()){
      Serial.println("Updating yiyan...");
      yiyan.updateSentences();
    }
    page.showPage();
  }
  todo = pref.getTodo();
  todoCount = parseTodoTasks(todo, todos);

}

void setup() {
  Serial.begin(115200);
  // E-paper初始化
  SPI.begin();  
  display.epd2.selectSPI(SPI, SPISettings(10000000, MSBFIRST, SPI_MODE0));

  display.init();
  delay(500);
  display.setRotation(3);
  display.setTextColor(GxEPD_BLACK);
  u8g2Fonts.begin(display);
  u8g2Fonts.setFontDirection(0);    // 设置文字显示方向  
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);   // 设置前景色
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);   // 设置背景色
  u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312); // 设置文本字体

  // 开机动画3秒
  onBootShow();
  // 按键初始化
  btnNext.begin();
  btnHome.begin();
  // 页面初始化
  page.begin();
  // Preference 加载
  pref.load();
  pref.showAllPreference();
  
  wifiManager.begin();
  networkStateShow();

  if (wifiManager.isConnected()){
    wifiManager.getNetworkTime(timeStr, sizeof(timeStr));
    Serial.print("Network Time: \t");
    Serial.println(timeStr);
    wifiManager.parseTimeString(timeStr, year, month, day, hour, minute, second, weekday, monthStr);
    // 依次打印每个字段
    Serial.print("year: "); Serial.println(year);
    Serial.print("month: "); Serial.println(month);
    Serial.print("day: "); Serial.println(day);
    Serial.print("hour: "); Serial.println(hour);
    Serial.print("minute: "); Serial.println(minute);
    Serial.print("second: "); Serial.println(second);
    Serial.print("weekday: "); Serial.println(weekday);
  // 更新天气
    if (!weather.updated){
      Serial.println("Updating weather...");
      Serial.println("pref load completed");
      String apiKey = pref.getApiKey();
      String location = pref.getLocationID();
      String locationName = pref.getLocationName();
      weather.reConfig(apiKey,location,locationName);
      weather.update();
      bool isDaytime = true; //默认白天 只有在开机的时候才判断是不是白天
      if (hour > 18){isDaytime = false;}
      weather.setParams(true,isDaytime); // 暂时都是获取今天白天数据
      weather.showWeather();
    }
    if (!(history.isupdated)) {
      Serial.println("Updating history...");
      history.updateHistory();  // 获取历史数据
    } 
    if (!(news.isUpdated)){
      Serial.println("Updating news...");
      news.updateNews();
      news.printAllNews();
    }
    if (!yiyan.isOnline()){
      Serial.println("Updating yiyan...");
      yiyan.updateSentences();
    }
  }
  todo = pref.getTodo();
  todoCount = parseTodoTasks(todo, todos);
  page.showPage();
}

void loop() {
// 按键事件检测
keyEvent = KEY_EVENT_NONE; //每次循环先将keyEvent重置为None
btnNext.update();
btnHome.update();
switch (btnHome.getEvent()) {
  case BUTTON_SINGLE: keyEvent = KEY_HOME_CLICK; Serial.println("Home SINGLE"); break;
  case BUTTON_LONG:   keyEvent = KEY_HOME_LONG; Serial.println("Home LONG");   break;
}
switch (btnNext.getEvent()) {
  case BUTTON_SINGLE: keyEvent = KEY_NEXT_CLICK; Serial.println("Next SINGLE"); break;
  case BUTTON_LONG:   keyEvent = KEY_NEXT_LONG; Serial.println("Next LONG");   break;
}
// 页面逻辑切换
if (page.handleKeyEvent(keyEvent))
{
  page.showPage();
}
if (pref.changed){
  if (page.getCurrentPage() == PAGE_TODO){
    page.showPage();
  }
  pref.changed = false;
}
//updateInfo();
}
