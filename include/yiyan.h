#ifndef YIYAN_H
#define YIYAN_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <vector>
#include <ArduinoJson.h>

class Yiyan {
private:
    std::vector<String> sentences;  // 存储10条句子
    int currentIndex;                // 当前返回的索引
    bool isConnected;                // 网络连接状态

public:
    // 构造函数
    Yiyan() : currentIndex(0), isConnected(false) {
        sentences = {
            "Cleverness is not wisdom...",
            "We need never be ashamed of our tears...",
            "Much wisdom often goes with fewest words...",
            "Our distrust is very expensive...",
            "The only way to have a friend is to be one..."
        };
    }

    // 检查 WiFi 连接
    bool checkConnection() {
        Serial.println(WiFi.status());
        isConnected = (WiFi.status() == WL_CONNECTED);
        return isConnected;
    }

    void updateSentences() {
        if (!checkConnection()) return;  // 没有网络则不更新
    
        WiFiClientSecure client;
        client.setInsecure();  // 忽略 SSL 证书检查
    
        HTTPClient http;
        http.addHeader("User-Agent", "Mozilla/5.0");  // 添加 User-Agent 头
    
        for (int i = 0; i < YIYANNUM; ++i) {
            http.begin(client, "https://api.quotable.io/random?minLength=50&maxLength=100");
    
            int httpCode = http.GET();  // 发送 GET 请求
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();  // 获取 JSON 响应

                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, payload);
    
                if (error) {
                    Serial.print("JSON 解析失败: ");
                    Serial.println(error.f_str());
                    continue;
                }
    
                // 提取句子
                const char* quote = doc["content"];
                sentences[i] = String(quote);  // 存入数组
    
                Serial.print("获取句子: ");
                Serial.println(sentences[i]);
            } else {
                Serial.printf("HTTP 请求失败, 错误码: %d\n", httpCode);
            }
    
            http.end();
            delay(200);  // 避免请求过快
        }
    }
    // 获取当前的一句话
    String getSentence() {
        if (sentences.empty()) return "NULL";
        String sentence = sentences[currentIndex];
        currentIndex = (currentIndex + 1) % sentences.size();
        return sentence;
    }

    // 获取网络状态
    bool isOnline() {
        return isConnected;
    }
};

#endif