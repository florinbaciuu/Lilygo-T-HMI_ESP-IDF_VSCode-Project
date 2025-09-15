#pragma once

#include "driver/gpio.h"
#include "esp_timer.h"
#include <stdint.h>
#include <stdbool.h>

typedef void (*callbackFunction)(void);
typedef void (*parameterizedCallbackFunction)(void*);

class OneButton {
public:
    OneButton();
    OneButton(gpio_num_t pin, bool activeLow = true, bool pullupActive = true);

    void setDebounceTicks(int ticks);
    void setClickTicks(int ticks);
    void setPressTicks(int ticks);

    void attachClick(callbackFunction newFunction);
    void attachClick(parameterizedCallbackFunction newFunction, void* parameter);

    void attachDoubleClick(callbackFunction newFunction);
    void attachDoubleClick(parameterizedCallbackFunction newFunction, void* parameter);

    void attachMultiClick(callbackFunction newFunction);
    void attachMultiClick(parameterizedCallbackFunction newFunction, void* parameter);

    void attachLongPressStart(callbackFunction newFunction);
    void attachLongPressStart(parameterizedCallbackFunction newFunction, void* parameter);

    void attachLongPressStop(callbackFunction newFunction);
    void attachLongPressStop(parameterizedCallbackFunction newFunction, void* parameter);

    void attachDuringLongPress(callbackFunction newFunction);
    void attachDuringLongPress(parameterizedCallbackFunction newFunction, void* parameter);

    void reset(void);
    int getNumberClicks(void);

    void tick(void);
    void tick(bool activeLevel);

private:
    typedef enum {
        OCS_INIT = 0,
        OCS_DOWN,
        OCS_UP,
        OCS_COUNT,
        OCS_PRESS,
        OCS_PRESSEND
    } stateMachine_t;

    void _newState(stateMachine_t nextState);

    gpio_num_t _pin;
    int _buttonPressed = 0;

    int _debounceTicks = 50;
    int _clickTicks = 400;
    int _pressTicks = 800;
    int _maxClicks = 1;

    stateMachine_t _state = OCS_INIT;
    stateMachine_t _lastState = OCS_INIT;

    int _nClicks = 0;
    uint32_t _startTime = 0;

    callbackFunction _clickFunc = nullptr;
    parameterizedCallbackFunction _paramClickFunc = nullptr;
    void* _clickFuncParam = nullptr;

    callbackFunction _doubleClickFunc = nullptr;
    parameterizedCallbackFunction _paramDoubleClickFunc = nullptr;
    void* _doubleClickFuncParam = nullptr;

    callbackFunction _multiClickFunc = nullptr;
    parameterizedCallbackFunction _paramMultiClickFunc = nullptr;
    void* _multiClickFuncParam = nullptr;

    callbackFunction _longPressStartFunc = nullptr;
    parameterizedCallbackFunction _paramLongPressStartFunc = nullptr;
    void* _longPressStartFuncParam = nullptr;

    callbackFunction _longPressStopFunc = nullptr;
    parameterizedCallbackFunction _paramLongPressStopFunc = nullptr;
    void* _longPressStopFuncParam = nullptr;

    callbackFunction _duringLongPressFunc = nullptr;
    parameterizedCallbackFunction _paramDuringLongPressFunc = nullptr;
    void* _duringLongPressFuncParam = nullptr;
};
