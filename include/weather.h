#ifndef _WEATHER_H
#define _WEATHER_H

#include "Arduino.h"
#include "ArduinoJson.h"
#include "ArduinoUZlib.h"
#include "HTTPClient.h"

//定义天气类
class Weather
{
    public:
        Weather(String apiKey = "", String location = "", String locationName = "");
        void reConfig(String apiKey, String location, String locationName);
        bool update();  //发送一次请求信息，获取天气数据
        int todayTempMax, todayTempMin, todayWeaId, IconId;
        bool updated = false;
        String todayWeaIcon;
        //只需要今明日的天气与温度，故仅设置了以下若干函数
        int getTempMax(bool is_today);
        int getTempMin(bool is_today);
        int getWeather(bool is_today, bool is_day);
        String WeatherIcon(int weatherid);
        void setParams(bool is_today, bool is_day);
        String getLocation(){return location;};
        String getLocaName(){return locaName;};
        void showWeather();
    private:
        String apiKey;                  //天气API密钥
        String location;                //地点
        String locaName;
        //StaticJsonDocument<2048> doc;   //Json容器
        JsonDocument doc;
        JsonObject today, tomorrow;     //可根据返回的json信息自行定义其他天气数据
        
};

#endif
