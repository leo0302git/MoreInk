#include"WifiManager.h"

void WifiManager::begin() {
    // Wifi初始化的时候，先加载Preference内容，并尝试用Preference中的wifi配置连接到互联网
    loadPreferences();
    Serial.println("wifi.begin() 尝试用Preference连接wifi");
    Serial.printf("ssid=%s\t",ssid.c_str());
    Serial.printf("password=%s\t",password.c_str());
    Serial.printf("apiKey=%s\t",apiKey.c_str());
    Serial.printf("locationName=%s\n",locationName.c_str());
    if (tryConnectSTA()) {
        Serial.println("已连接到WiFi");
    } else {
        // 如果Preference中的wifi不可连接，开启AP模式
        startAP();
    }
    // 此时AP模式已经打开
    // 通用路由
    // ESP32作为服务器，展示表单，供手机提交
    server.on("/", [this]() { server.send(200, "text/html", formHTML); });
    // 监听手机行为，handleSave处理手机提交的数据。既更新Preference，又尝试连接互联网
    // 一旦打开AP模式（不管是wifi.begin()还是wifi.loop()中打开的），提交表单后都会触发handleSave，以此达到“传入新配置后自动重连”的功能
    server.on("/save", HTTP_POST, [this]() { handleSave(); });
    server.begin();
}

void WifiManager::loop() {
    server.handleClient();
    if (!WiFi.isConnected() && WiFi.getMode() == WIFI_STA) {
        //tryConnectSTA();
        if (WiFi.status() != WL_CONNECTED) {
            connected = false;
            Serial.println("WiFi 断开，进入AP模式...");
            startAP();
        }
    }
}

void WifiManager::loadPreferences(){
    Serial.println("wifi module loading pref...");
    pref.load();
    ssid = pref.getSSID();
    password = pref.getPassword();
    apiKey = pref.getApiKey();
    locationName = pref.getLocationName();


    // 加载新字段
    morningTime   = pref.getMorningTime();
    noonTime      = pref.getNoonTime();
    eveningTime   = pref.getEveningTime();
    todo          = pref.getTodo();
}

void WifiManager::savePreferences() {
    Serial.println("Saving Preference...");
    pref.applyAndCheckChanges(ssid,password, apiKey, locationName, locationID, morningTime, noonTime, eveningTime, todo);
    if (pref.changed) Serial.println("wifi module changed pref.");
    else Serial.println("wifi module didn't change pref.");
}


bool WifiManager::tryConnectSTA(){
    Serial.print("tryConnectSTA: ");
    Serial.printf("ssid: %s, password: %s\n",ssid, password);
    if (ssid == "" || password == "") return false;

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.printf("尝试连接 WiFi: %s ...\n", ssid.c_str());

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nWiFi连接成功，IP地址: %s\n", WiFi.localIP().toString().c_str());
        connected = true;
        localIP = WiFi.localIP().toString();
        return true;
    }
    Serial.println("\nWiFi连接失败");
    connected = false;
    return false;
}

void WifiManager::startAP(){
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MoreInk-ESP32","11223333");
    Serial.println("AP模式开启，SSID: MoreInk-ESP32 密码: 11223333");
    Serial.println("请用其他设备访问 http://192.168.4.1 (关闭代理或VPN，否则可能连接ESP32失败)");
}

void WifiManager::handleSave(){
    // 从网页提取用户通过手机传递的信息
    ssid     = server.arg("ssid");
    password = server.arg("password");
    apiKey   = server.arg("apikey");
    locationName = server.arg("locationName");
    // 新增字段
    morningTime   = server.arg("morningTime");  // 格式 "HH:MM"
    noonTime      = server.arg("noonTime");
    eveningTime   = server.arg("eveningTime");
    todo          = server.arg("todo");

    Serial.println("wifi.handleSave() 收到的字段：");
    Serial.printf("ssid=%s\t",ssid.c_str());
    Serial.printf("password=%s\t",password.c_str());
    Serial.printf("apiKey=%s\t",apiKey.c_str());
    Serial.printf("locationName=%s\n",locationName.c_str());
    Serial.printf("morning=%s\t", morningTime.c_str());
    Serial.printf("noon=%s\t",noonTime.c_str());
    Serial.printf("evening=%s\t", eveningTime.c_str());
    Serial.printf("todo=%s\t", todo.c_str());

    Serial.println("收到新配置，尝试连接...");
    server.send(200, "text/html", "配置已保存，正在尝试连接WiFi...");

    savePreferences(); // 这里存储会只存储非空字段
    loadPreferences();
    // 先断开再连接
    WiFi.disconnect();
    Serial.println("5秒后尝试用新配置连接wifi");
    delay(1000*5);
    delay(500);
    if (tryConnectSTA()) {
        Serial.println("新配置连接成功！");
    } else {
        Serial.println("新配置连接失败，回到AP模式");
        startAP();
    }
}
void WifiManager::getNetworkTime(char *buffer, size_t bufferSize)
{
    const char* ntpServer1 = "pool.ntp.org";
    const char* ntpServer2 = "time.nist.gov";
    const long  gmtOffset_sec = 8 * 3600; // 东八区
    const int   daylightOffset_sec = 0;

    if (WiFi.status() != WL_CONNECTED) {
        snprintf(buffer, bufferSize, "WiFi not connected");
        return;
    }

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
    Serial.println("Fetching network time...");

    struct tm timeinfo;
    int retry = 0;
    const int maxRetry = 5;
    while (!getLocalTime(&timeinfo) && retry < maxRetry) {
        Serial.printf("Waiting for NTP sync... (%d/%d)\n", retry + 1, maxRetry);
        delay(1000);
        retry++;
    }

    if (retry == maxRetry) {
        snprintf(buffer, bufferSize, "Failed to get NTP time");
        return;
    }

    strftime(buffer, bufferSize, "%A, %B %d %Y %H:%M:%S", &timeinfo);
}


void WifiManager::getLocalTimeString(char* buffer, size_t len) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        strncpy(buffer, "No time available", len);
        return;
    }
    strftime(buffer, len, "%Y-%m-%d %H:%M:%S", &timeinfo); // 格式化时间
}



// **月份映射**
const char* months[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};
// **解析时间字符串**
void WifiManager::parseTimeString(const char* timeStr, int &year, int &month, int &day, int &hour, int &minute, int &second, char* weekday, char* monthStr) {
    char buffer[64];  // **复制 timeStr，防止修改原始字符串**
    strncpy(buffer, timeStr, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* token = strtok(buffer, " ,");  // **按空格和逗号分割**
    
    // **提取星期**
    if (token) {
        strncpy(weekday, token, 10);
        weekday[9] = '\0';  // 确保字符串结尾
    }
    
    // **提取月份**
    token = strtok(NULL, " ,");
    strncpy(monthStr, token, 10);  // 保存月份字符串
    monthStr[9] = '\0';
    if (token) {
        for (int i = 0; i < 12; i++) {
            if (strcmp(token, months[i]) == 0) {
                month = i + 1;
                break;
            }
        }
    }

    // **提取日期、年份**
    token = strtok(NULL, " ,");
    if (token) day = atoi(token);

    token = strtok(NULL, " ,");
    if (token) year = atoi(token);

    // **提取时间（小时、分钟、秒）**
    token = strtok(NULL, ":");
    if (token) hour = atoi(token);

    token = strtok(NULL, ":");
    if (token) minute = atoi(token);

    token = strtok(NULL, ":");
    if (token) second = atoi(token);
}
void WifiManager::parseLocalTime(const char* localtime, int &year, int &month, int &day, int &hour, int &minute) {
    // **使用 sscanf 解析字符串**
    if (sscanf(localtime, "%d-%d-%d %d:%d", &year, &month, &day, &hour, &minute) == 5) {
        //Serial.printf("📅 解析成功！\n");
    } else {
        //Serial.println("⚠️ 解析失败，请检查时间格式！");
    }
}