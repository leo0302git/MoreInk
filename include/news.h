#ifndef NEWS_H
#define NEWS_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <vector>
#include <ArduinoJson.h>
#include "config.h"

class News {
private:
    std::vector<String> newsList;  // 存储新闻列表
    int currentIndex;              // 当前索引
    bool isConnected;              // 网络连接状态
    
public:
    // 构造函数
    bool isUpdated;
    News() : currentIndex(0), isConnected(false) {
        newsList.resize(NEWSNUM, "暂无新闻");  // 预设默认新闻
        isUpdated = false;
    }

    // 检查 WiFi 连接
    bool checkConnection() {
        isConnected = (WiFi.status() == WL_CONNECTED);
        return isConnected;
    }

    // 更新新闻
    void updateNews();

    // 获取一条新闻
    String getNews();

    // 获取网络状态
    bool isOnline();

    void printAllNews();
};

#endif
