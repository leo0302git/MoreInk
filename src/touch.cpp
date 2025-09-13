#include "touch.h"

// 静态表初始化
TouchButton* TouchButton::instances[10] = { nullptr };

TouchButton::TouchButton(uint8_t pin,
                         uint16_t threshold,
                         unsigned long longPressMs,
                         uint16_t hysteresis,
                         unsigned long debounceIsrMs,
                         unsigned long minPressMs,
                         unsigned long releaseStableMs,
                        uint8_t idx)
: _pin(pin),
  _threshold(threshold),
  _hysteresis(hysteresis),
  _releaseThreshold(threshold + hysteresis),
  _longPressMs(longPressMs),
  _debounceIsrMs(debounceIsrMs),
  _minPressMs(minPressMs),
  _releaseStableMs(releaseStableMs),
  _idx(idx) {}

void TouchButton::begin() {
    // 记录实例
    // if (_pin < 100) {
        
    // }
    instances[_idx] = this;
    // 绑定对应的 ISR（Arduino-ESP32 的 touchAttachInterrupt 只接受 void (*)()）
    switch (_pin) {
        case 4: touchAttachInterrupt(T0, isrHandler0, _threshold); break;
        case 1: touchAttachInterrupt(T1, isrHandler1, _threshold); break;
        case 2: touchAttachInterrupt(T2, isrHandler2, _threshold); break;
        case 3: touchAttachInterrupt(T3, isrHandler3, _threshold); break;
        //case 4: touchAttachInterrupt(T4, isrHandler4, _threshold); break;
        case 5: touchAttachInterrupt(T5, isrHandler5, _threshold); break;
        case 6: touchAttachInterrupt(T6, isrHandler6, _threshold); break;
        case 7: touchAttachInterrupt(T7, isrHandler7, _threshold); break;
        case 8: touchAttachInterrupt(T8, isrHandler8, _threshold); break;
        case 32: touchAttachInterrupt(T9, isrHandler9, _threshold); break;
        default: /* 非法通道，不绑定 */ break;
    }
}

// —— 各通道 ISR：只做转发到实例方法，严禁做重活/打印 —— //
void IRAM_ATTR TouchButton::isrHandler0(){ if (instances[0]) instances[0]->handleISR(); }
void IRAM_ATTR TouchButton::isrHandler1(){ if (instances[1]) instances[1]->handleISR(); }
void IRAM_ATTR TouchButton::isrHandler2(){ if (instances[2]) instances[2]->handleISR(); }
void IRAM_ATTR TouchButton::isrHandler3(){ if (instances[3]) instances[3]->handleISR(); }
void IRAM_ATTR TouchButton::isrHandler4(){ if (instances[4]) instances[4]->handleISR(); }
void IRAM_ATTR TouchButton::isrHandler5(){ if (instances[5]) instances[5]->handleISR(); }
void IRAM_ATTR TouchButton::isrHandler6(){ if (instances[6]) instances[6]->handleISR(); }
void IRAM_ATTR TouchButton::isrHandler7(){ if (instances[7]) instances[7]->handleISR(); }
void IRAM_ATTR TouchButton::isrHandler8(){ if (instances[8]) instances[8]->handleISR(); }
void IRAM_ATTR TouchButton::isrHandler9(){ if (instances[9]) instances[9]->handleISR(); }

// 实际 ISR 处理：只做“按下边沿”的可靠识别 + 去抖
void IRAM_ATTR TouchButton::handleISR() {
    unsigned long now = millis(); // 简短可用；避免 Serial/动态分配

    // 中断去抖：同一按键两次触发需间隔 _debounceIsrMs
    if (now - _lastIsrTime < _debounceIsrMs) return;
    _lastIsrTime = now;

    // 仅第一次有效按下生效
    if (!_pressedFlag) {
        _pressedFlag = true;
        _pressTime = now;
        _longPressDetected = false;
        _releaseStartMs = 0;
        // 不在 ISR 里打印或发布事件
    }
}

void TouchButton::update() {
    // 只在“已被认为按下”的状态下进行判定
    if (_pressedFlag) {
        unsigned long now = millis();
        
        // 1) 长按到点即判定（只触发一次）
        if (!_longPressDetected && (now - _pressTime >= _longPressMs)) {
            _event = BUTTON_LONG;
            _longPressDetected = true;
        }

        // 2) 释放判定：需要触摸值持续高于 “释放阈值=阈值+hysteresis”
        int v = touchRead(_pin);

        if (v > _releaseThreshold) {
            if (_releaseStartMs == 0) _releaseStartMs = now;

            // 释放需要“稳定时间”，并且按压时长要至少达到最小有效按压时间
            if ((now - _releaseStartMs >= _releaseStableMs) &&
                (now - _pressTime      >= _minPressMs)) {

                if (!_longPressDetected) {
                    // 从未触发过长按 => 这是单击
                    _event = BUTTON_SINGLE;
                }
                // 复位状态，等待下一次按下
                _pressedFlag = false;
                _releaseStartMs = 0;
            }
        } else {
            // 仍在按下侧，清空释放计时
            _releaseStartMs = 0;
        }
    }
}

ButtonEvent TouchButton::getEvent() {
    ButtonEvent e = _event;
    _event = BUTTON_NONE;
    return e;
}
