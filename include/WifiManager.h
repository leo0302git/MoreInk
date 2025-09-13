#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include <WebServer.h>
//#include <Preferences.h>
#include "PrefManager.h"
class WifiManager {
private:
    WebServer server;
    //Preferences preferences;
    PrefManager pref;
    
    String ssid;
    String password;
    String apiKey;
    String locationName;
    String locationID;
    String localIP = "192.168.4.1";

    String morningTime = "08:00";
    String noonTime = "11:30";
    String eveningTime = "06:00";
    String todo = "无代办事项";
    bool connected = false;

    // 配置页面HTML
    const char* formHTML = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head><meta charset="UTF-8"><title>ESP32 配置</title></head>
    <body>
      <h2>ESP32 参数配置</h2>
      <form action="/save" method="post">
        WiFi SSID: <input type="text" name="ssid"><br><br>
        WiFi 密码: <input type="password" name="password"><br><br>
        和风天气位置ID: <input type="text" name="locationID"><br><br>
        和风天气API Key: <input type="text" name="apikey"><br><br>
        位置名称: <input type="text" name="locationName"><br><br>
        <!-- 定时输入框 -->
        早上任务时间: <input type="time" name="morningTime"><br><br>
        中午任务时间: <input type="time" name="noonTime"><br><br>
        晚上任务时间: <input type="time" name="eveningTime"><br><br>

        <!-- 代办事项输入框 -->
        代办事项: <input type="text" name="todo"><br><br>
        <input type="submit" value="保存配置">
      </form>
    </body>
    </html>
    )rawliteral";

public:
    WifiManager() : server(80) {}

    void begin();
    bool isConnected(){connected = WiFi.isConnected(); return connected;};
    String getIP(){
      localIP = WiFi.localIP().toString(); 
      if (localIP == "0.0.0.0"){
        localIP = "192.168.4.1";
      }
      return localIP;
    };
    void loop();
    void getNetworkTime(char *buffer, size_t bufferSize);
    void getLocalTimeString(char* buffer, size_t len);
    void parseTimeString(const char* timeStr, int &year, int &month, int &day, int &hour, int &minute, int &second, char* weekday, char* monthStr) ;
    void parseLocalTime(const char* localtime, int &year, int &month, int &day, int &hour, int &minute);
    bool tryConnectSTA();
private:
    void loadPreferences();

    void savePreferences();

    void startAP();

    void handleSave();
};

#endif