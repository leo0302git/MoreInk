#ifndef PREF_MANAGER_H
#define PREF_MANAGER_H

#include <Preferences.h>
#include <Arduino.h>

class PrefManager {
public:
    PrefManager() {}
    ~PrefManager() {}
    bool changed = false;
    // 加载 Preferences
    void load() {
        Serial.println("pref is loading...");
        preferences.begin("config", true);
        Serial.printf("freeEntries: %d\n", preferences.freeEntries());

        if (preferences.isKey("count")) {
            uint32_t count = preferences.getUInt("count", 0);
            count++;
            Serial.printf("这是系统第 %u 次启动\n", count);
            preferences.putUInt("count", count);
        }

        ssid          = preferences.getString("ssid", "");
        password      = preferences.getString("password", "");
        apiKey        = preferences.getString("apikey", "");
        locationName  = preferences.getString("locationName", "");
        locationID    = preferences.getString("locationID", "");
        morningTime   = preferences.getString("morningTime", "08:00");
        noonTime      = preferences.getString("noonTime", "11:30");
        eveningTime   = preferences.getString("eveningTime", "18:00");
        todo          = preferences.getString("todo", "无代办事项");

        Serial.println("closing preference...");
        preferences.end();
    }

    // 安全保存：只有新值非空才覆盖，否则保留旧值
    void saveWithFallback() {
        preferences.begin("config", false);

        safePut("ssid", ssid);
        safePut("password", password);
        safePut("apikey", apiKey);
        safePut("locationName", locationName);
        safePut("locationID", locationID);
        safePut("morningTime", morningTime);
        safePut("noonTime", noonTime);
        safePut("eveningTime", eveningTime);
        safePut("todo", todo);

        preferences.end();
    }

    // 应用新值并检测是否有任何字段变化
    bool applyAndCheckChanges(
        const String& newSSID,
        const String& newPassword,
        const String& newApiKey,
        const String& newLocationName,
        const String& newLocationID,
        const String& newMorningTime,
        const String& newNoonTime,
        const String& newEveningTime,
        const String& newTodo
    ) {
        changed = false;

        changed |= updateField(ssid, newSSID);
        changed |= updateField(password, newPassword);
        changed |= updateField(apiKey, newApiKey);
        changed |= updateField(locationName, newLocationName);
        changed |= updateField(locationID, newLocationID);
        changed |= updateField(morningTime, newMorningTime);
        changed |= updateField(noonTime, newNoonTime);
        changed |= updateField(eveningTime, newEveningTime);
        changed |= updateField(todo, newTodo);

        if (changed) {
            saveWithFallback();
        }

        return changed;
    }

    // Getter
    String getSSID()         { return ssid; }
    String getPassword()     { return password; }
    String getApiKey()       { return apiKey; }
    String getLocationName() { return locationName; }
    String getLocationID()   { return locationID; }
    String getMorningTime()  { return morningTime; }
    String getNoonTime()     { return noonTime; }
    String getEveningTime()  { return eveningTime; }
    String getTodo()         { return todo; }

    // Setter
    void setSSID(const String& v)         { ssid = v; }
    void setPassword(const String& v)     { password = v; }
    void setApiKey(const String& v)       { apiKey = v; }
    void setLocationName(const String& v) { locationName = v; }
    void setLocationID(const String& v)   { locationID = v; }
    void setMorningTime(const String& v)  { morningTime = v; }
    void setNoonTime(const String& v)     { noonTime = v; }
    void setEveningTime(const String& v)  { eveningTime = v; }
    void setTodo(const String& v)         { todo = v; }
    void showAllPreference() {
        Serial.println("所有偏好设置信息：");
        Serial.println("---------------------");
        Serial.print("SSID: ");
        Serial.println(getSSID());
        Serial.print("密码: ");
        Serial.println(getPassword());
        Serial.print("API密钥: ");
        Serial.println(getApiKey().c_str());
        Serial.print("位置名称: ");
        Serial.println(getLocationName());
        Serial.print("位置ID: ");
        Serial.println(getLocationID());
        Serial.print("上午时间: ");
        Serial.println(getMorningTime());
        Serial.print("中午时间: ");
        Serial.println(getNoonTime());
        Serial.print("晚上时间: ");
        Serial.println(getEveningTime());
        Serial.print("待办事项: ");
        Serial.println(getTodo());
        Serial.println("---------------------");
    }
private:
    Preferences preferences;

    String ssid;
    String password;
    String apiKey;
    String locationName;
    String locationID;

    String morningTime  = "08:00";
    String noonTime     = "11:30";
    String eveningTime  = "18:00";
    String todo         = "无代办事项";

    // 仅在 value 非空时写入，否则保留原值
    void safePut(const char* key, String &value) {
        String oldVal = preferences.getString(key, "");
        if (!value.isEmpty()) {
            preferences.putString(key, value);
        } else {
            value = oldVal; // 保留旧值
        }
    }

    // 比较并更新字段，返回是否发生变化
    bool updateField(String &field, const String &newVal) {
        if (!newVal.isEmpty() && newVal != field) {
            field = newVal;
            return true;
        }
        return false;
    }
};

#endif
