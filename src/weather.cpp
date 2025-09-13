#include "weather.h"


Weather::Weather(String apiKey, String location, String locationName)
{
    this->apiKey = apiKey;
    this->location = location;
    this->locaName = locationName;
    this->todayTempMax = -100;
    this->todayTempMin = -100;
    this->todayWeaId = -1;
    this->todayWeaIcon = "UNKNOWN WAETHER";
    this->IconId = -1; // 适配 open_iconic_weather_4x
}

void Weather::reConfig(String apiKey, String location, String locationName)
{
    Serial.println("reConfiguring weather...");
    this->apiKey = apiKey;
    this->location = location;
    this->locaName = locationName;
}

bool Weather::update()
{
    HTTPClient http;   //用于访问网络
    WiFiClient *stream;
    int size;
    
    http.begin("https://devapi.qweather.com/v7/weather/3d?location="+ this->location + "&key=" + this->apiKey); //获取近三天天气信息
    int httpcode = http.GET();   //发送GET请求
    if(httpcode > 0)
    {
        if(httpcode == HTTP_CODE_OK)
        {
            stream = http.getStreamPtr();   //读取服务器返回的数据
            size = http.getSize();
            Serial.printf("weather http size: %d\n", size);
            if (size <= 0) {
                Serial.println("Invalid HTTP content size!");
                http.end();
                return false;
            }
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpcode).c_str());
    }
    http.end();   //结束当前连接

    uint8_t* inbuff = (uint8_t*) malloc(size);
    if (!inbuff) {
        Serial.println("Malloc failed!");
        return false;
    }
    stream->readBytes(inbuff, size);
    // uint8_t inbuff[size];
    // stream->readBytes(inbuff, size);
    uint8_t *outbuf = NULL;
    uint32_t out_size = 0;
    int result = ArduinoUZlib::decompress(inbuff,size, outbuf, out_size);
    deserializeJson(doc, outbuf);
    today = doc["daily"][0].as<JsonObject>();
    tomorrow = doc["daily"][1].as<JsonObject>();
    updated = true;
    return true;
}

//获取天气数据中的当天最高气温数据
int Weather::getTempMax(bool is_today)
{
    if(is_today)
    {
        return today["tempMax"].as<int>();
    }
    else
    {
        return tomorrow["tempMax"].as<int>();
    }
}

//获取天气数据中的当天最低气温数据
int Weather::getTempMin(bool is_today)
{
    if(is_today)
    {
        return today["tempMin"].as<int>();
    }
    else
    {
        return tomorrow["tempMin"].as<int>();
    }
}

//获取天气数据中的天气标识代码
int Weather::getWeather(bool is_today, bool is_day)
{
    if(is_today)
    {
        if(is_day)
            return today["iconDay"].as<int>();
        else
            return today["iconNight"].as<int>();
    }
    else
    {
        if(is_day)
            return tomorrow["iconDay"].as<int>();
        else
            return tomorrow["iconNight"].as<int>();
    }
}


//根据和风天气API返回天气标识
String Weather::WeatherIcon(int weatherid)
{
    String res = "未知天气";
    switch(weatherid)
    {
        case 100://白天晴
            res = "白天晴";
            IconId = 69;
            break;
        case 101://多云
        case 102://少云
        case 151://夜间多云
        case 152://夜间少云
        case 103://晴间多云
            res = "多云";
            IconId = 65;
            break;
        case 104://阴天
            res = "阴天";
            IconId = 64;
            break;
        case 150://夜间晴朗
            res = "夜间晴朗";
            IconId = 66;
            break;
        case 153://夜间晴间多云
            res = "夜间多云";
            IconId = 65;
            break;
        case 305://小雨
        case 309://毛毛雨/细雨
            res = "小雨";
            IconId = 67;            
            break;
        case 300://阵雨
        case 350://夜间阵雨
        case 306://中雨
        case 399://雨
        case 313://冻雨
        case 314://小到中雨
            res = "阵雨";
            IconId = 67;
            break;
        case 301://强阵雨
        case 351://夜间强阵雨
        case 307://大雨
        case 308://极端降雨
        case 310://暴雨
        case 311://大暴雨
        case 312://特大暴雨
        case 315://中到大雨
        case 316://大到暴雨
        case 317://暴雨到大暴雨
        case 318://大暴雨到特大暴雨
            res = "大雨";
            IconId = 67;
            break;
        case 302://雷阵雨
        res = "雷阵雨";
        IconId = 67;
            break;
        case 303://强雷阵雨
        res = "强雷阵雨";
        IconId = 67;
            break;

        //雪天
        case 400://小雪
        case 401://中雪
        case 402://大雪
        case 403://暴雪
        case 404://雨夹雪
        case 405://雨雪天气
        case 406://阵雨夹雪
        case 407://阵雨夹雪
        case 408://小到中雪
        case 409://中到大雪
        case 410://雪
        case 456://阵雨夹雪
        case 457://阵雪
        case 499://雪
        res = "雪天";
            Serial.println("雪天\r\n");
            break;

        //雾霾
        case 500://薄雾
        case 501://雾
        case 509://浓雾
        case 510://强浓雾
        case 514://大雾
        case 515://特强浓雾
        res = "雾天";
            Serial.println("雾天\r\n");
            break;
        case 502://霾
        case 511://中度霾
        case 512://重度霾
        case 513://严重霾
        res = "雾霾";
            Serial.println("雾霾\r\n");
            break;
    }
    return res;
}

void Weather::setParams(bool is_today, bool is_day)
{
    Serial.println("setting Params...");
    todayTempMax = this->getTempMax(is_today);
    todayTempMin = this->getTempMin(is_today);
    todayWeaId = this->getWeather(!is_today,is_day);
    todayWeaIcon = this->WeatherIcon(todayWeaId);
}

void Weather::showWeather()
{
    // 如果一次性串行打印太长，应该像下面一样分为多个Serial来打印，方便维护
    Serial.println("showing Weather...");
    Serial.printf("apiKey: %s\n", this->apiKey);
    Serial.printf("locationID: %s\n", this->location);
    Serial.printf("locationName: %s\n", this->locaName);
    Serial.printf("todayTempMax: %d\n", this->todayTempMax);
    Serial.printf("todayTempMin: %d\n", this->todayTempMin);
    Serial.printf("todayWeatherID: %d\n", this->todayWeaId);
    Serial.printf("todayWeatherIcon: %s\n", this->todayWeaIcon);
    Serial.printf("IconId: %d\n", this->IconId);
}
