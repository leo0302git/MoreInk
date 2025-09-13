#ifndef HISTORY_TODAY_H
#define HISTORY_TODAY_H

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"


class HistoryToday {
private:
    String events[HISTORY_NUM];  // 存储 10 条历史事件
    bool networkAvailable;       // 是否联网状态
    int num;

public:
    bool isupdated;
    HistoryToday() {
        num = HISTORY_NUM;
        isupdated = false;
        for (int i = 0; i < HISTORY_NUM; i++) {
            events[i] = "暂无数据";  // 初始化默认值
        }
        networkAvailable = false;
    }
    int numOfHistory(){return num;}
    bool checkConnection() {
        return WiFi.status() == WL_CONNECTED;
    }

    void updateHistory() {
        if (!checkConnection()) {
            networkAvailable = false;
            return;
        }

        networkAvailable = true;
        WiFiClientSecure client;
        client.setInsecure();  // 忽略 SSL 证书检查

        HTTPClient http;
        http.begin(client, "https://v2.xxapi.cn/api/history");
        http.addHeader("User-Agent", "Mozilla/5.0");

        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("历史数据: " + payload);

            // 解析 JSON
            //StaticJsonDocument<1024> doc;
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (error) {
                Serial.println("JSON 解析失败");
                return;
            }

            JsonArray data = doc["data"];
            for (int i = 0; i < HISTORY_NUM && i < data.size(); i++) {
                events[i] = data[i].as<String>();  // 存储事件
            }
        } else {
            Serial.printf("<历史上的今天> HTTP 请求失败, 错误码: %d\n", httpCode);
        }

        http.end();
        isupdated = true;
    }

    String getEvent(int index) {
        if (index < 0 || index >= HISTORY_NUM) {
            return "索引超出范围";
        }
        return events[index];
    }

    bool isNetworkAvailable() {
        return networkAvailable;
    }
};

#endif
