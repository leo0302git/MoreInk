#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>
#include "config.h"


class TouchButton {
public:
    // hysteresis: 释放判定的回差；debounceIsrMs: 中断去抖；
    // minPressMs: 有效按压的最小持续；releaseStableMs: 释放需要连续稳定时间
    TouchButton(uint8_t pin,
                uint16_t threshold      = 40,
                unsigned long longPressMs = 1000,
                uint16_t hysteresis     = 8,
                unsigned long debounceIsrMs  = 120,
                unsigned long minPressMs     = 50,
                unsigned long releaseStableMs= 30,
                uint8_t idx = 0 );

    void begin();
    void update();           // 在 loop() 里高频调用
    ButtonEvent getEvent();  // 取一次清一次

    int rawRead() const { return touchRead(_pin); } // 可选：调试用

private:
    uint8_t _pin;
    uint16_t _threshold;
    uint16_t _hysteresis;
    uint16_t _releaseThreshold;   // _threshold + _hysteresis
    unsigned long _longPressMs;
    unsigned long _debounceIsrMs;
    unsigned long _minPressMs;
    unsigned long _releaseStableMs;
    uint8_t _idx;

    // ISR/状态
    volatile bool _pressedFlag = false;     // “正在按下（有效）”状态
    volatile unsigned long _pressTime = 0;  // 首次有效按下时间戳
    volatile unsigned long _lastIsrTime = 0;// ISR 去抖
    unsigned long _releaseStartMs = 0;      // 释放稳定性计时
    bool _longPressDetected = false;        // 长按是否已判定
    ButtonEvent _event = BUTTON_NONE;

    void IRAM_ATTR handleISR();             // 由各通道 ISR 转发到这里

    // --- 静态实例表 + 10 个 ISR 适配（T0~T9） ---
    static TouchButton* instances[10];
    static void IRAM_ATTR isrHandler0();
    static void IRAM_ATTR isrHandler1();
    static void IRAM_ATTR isrHandler2();
    static void IRAM_ATTR isrHandler3();
    static void IRAM_ATTR isrHandler4();
    static void IRAM_ATTR isrHandler5();
    static void IRAM_ATTR isrHandler6();
    static void IRAM_ATTR isrHandler7();
    static void IRAM_ATTR isrHandler8();
    static void IRAM_ATTR isrHandler9();
};

#endif
