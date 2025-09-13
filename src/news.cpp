#include "news.h"

#define NEWS_API "https://news.ravelloh.top/latest.json"

void News::updateNews() {
    if (!checkConnection()) return;  // 无网络时不更新

    WiFiClientSecure client;
    client.setInsecure();  // 忽略 SSL 证书检查

    HTTPClient http;
    http.begin(client, NEWS_API);
    http.addHeader("User-Agent", "Mozilla/5.0");

    int httpCode = http.GET();  // 发送 GET 请求
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("✅ 获取到新闻 JSON 数据！");
        
        // 解析 JSON
        //StaticJsonDocument<8192> doc;
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print("❌ JSON 解析失败: ");
            Serial.println(error.f_str());
            return;
        }

        // 解析新闻内容
        JsonArray content = doc["content"];
        int newsCount = (NEWSNUM > content.size())?  content.size(): NEWSNUM;  // 限制最多 `num` 条新闻

        // 清空旧新闻
        newsList.clear();
        newsList.resize(NEWSNUM, "暂无新闻");

        // 存储新闻
        for (int i = 0; i < newsCount; i++) {
            newsList[i] = content[i].as<String>();
        }

        Serial.printf("✅ 成功获取 %d 条新闻！\n", newsCount);
    } else {
        Serial.printf("❌ HTTP 请求失败，错误码: %d\n", httpCode);
    }

    http.end();
    isUpdated = true;
}

// 获取一条新闻
String News::getNews() {
    if (newsList.empty()) return "暂无新闻";
    String news = newsList[currentIndex];
    currentIndex = (currentIndex + 1) % newsList.size();
    return news;
}

// 获取网络状态
bool News::isOnline() {
    return isConnected;
}

void News::printAllNews()
{
    if (newsList.empty()) {Serial.println("printAllNews 暂无新闻") ; return;};
    for(int i=0;i<newsList.size();i++){
        Serial.printf("第 %d 条新闻：",i+1);
        Serial.println(this->getNews());
    }
    Serial.println("printAllNews 结束");
}
