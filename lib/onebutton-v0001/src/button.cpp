#include "button.h"
#include "esp_log.h"

// ----- Initialization -----

OneButton::OneButton() {
    _pin = GPIO_NUM_NC;
}

OneButton::OneButton(gpio_num_t pin, bool activeLow, bool pullupActive) {
    _pin = pin;

    if (activeLow) {
        _buttonPressed = 0;
    } else {
        _buttonPressed = 1;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = pullupActive ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);
}

// ----- Configuration -----

void OneButton::setDebounceTicks(const int ticks) {
    _debounceTicks = ticks;
}
void OneButton::setClickTicks(const int ticks) {
    _clickTicks = ticks;
}
void OneButton::setPressTicks(const int ticks) {
    _pressTicks = ticks;
}

// ----- Attach callbacks -----

void OneButton::attachClick(callbackFunction newFunction) {
    _clickFunc = newFunction;
}
void OneButton::attachClick(parameterizedCallbackFunction newFunction, void* parameter) {
    _paramClickFunc = newFunction;
    _clickFuncParam = parameter;
}
void OneButton::attachDoubleClick(callbackFunction newFunction) {
    _doubleClickFunc = newFunction;
    _maxClicks       = (_maxClicks > 2) ? _maxClicks : 2;
}
void OneButton::attachDoubleClick(parameterizedCallbackFunction newFunction, void* parameter) {
    _paramDoubleClickFunc = newFunction;
    _doubleClickFuncParam = parameter;
    _maxClicks            = (_maxClicks > 2) ? _maxClicks : 2;
}
void OneButton::attachMultiClick(callbackFunction newFunction) {
    _multiClickFunc = newFunction;
    _maxClicks      = (_maxClicks > 100) ? _maxClicks : 100;
}
void OneButton::attachMultiClick(parameterizedCallbackFunction newFunction, void* parameter) {
    _paramMultiClickFunc = newFunction;
    _multiClickFuncParam = parameter;
    _maxClicks           = (_maxClicks > 100) ? _maxClicks : 100;
}
void OneButton::attachLongPressStart(callbackFunction newFunction) {
    _longPressStartFunc = newFunction;
}
void OneButton::attachLongPressStart(parameterizedCallbackFunction newFunction, void* parameter) {
    _paramLongPressStartFunc = newFunction;
    _longPressStartFuncParam = parameter;
}
void OneButton::attachLongPressStop(callbackFunction newFunction) {
    _longPressStopFunc = newFunction;
}
void OneButton::attachLongPressStop(parameterizedCallbackFunction newFunction, void* parameter) {
    _paramLongPressStopFunc = newFunction;
    _longPressStopFuncParam = parameter;
}
void OneButton::attachDuringLongPress(callbackFunction newFunction) {
    _duringLongPressFunc = newFunction;
}
void OneButton::attachDuringLongPress(parameterizedCallbackFunction newFunction, void* parameter) {
    _paramDuringLongPressFunc = newFunction;
    _duringLongPressFuncParam = parameter;
}

// ----- State machine -----

void OneButton::reset(void) {
    _state     = OCS_INIT;
    _lastState = OCS_INIT;
    _nClicks   = 0;
    _startTime = 0;
}

int OneButton::getNumberClicks(void) {
    return _nClicks;
}

void OneButton::tick(void) {
    if (_pin != GPIO_NUM_NC) {
        int level = gpio_get_level(_pin);
        tick(level == _buttonPressed);
    }
}

void OneButton::_newState(stateMachine_t nextState) {
    _lastState = _state;
    _state     = nextState;
}

void OneButton::tick(bool activeLevel) {
    uint32_t now      = (uint32_t) (esp_timer_get_time() / 1000);
    uint32_t waitTime = now - _startTime;

    switch (_state) {
        case OCS_INIT:
            if (activeLevel) {
                _newState(OCS_DOWN);
                _startTime = now;
                _nClicks   = 0;
            }
            break;

        case OCS_DOWN:
            if (!activeLevel && (waitTime < _debounceTicks)) {
                _newState(_lastState);
            } else if (!activeLevel) {
                _newState(OCS_UP);
                _startTime = now;
            } else if (activeLevel && (waitTime > _pressTicks)) {
                if (_longPressStartFunc)
                    _longPressStartFunc();
                if (_paramLongPressStartFunc)
                    _paramLongPressStartFunc(_longPressStartFuncParam);
                _newState(OCS_PRESS);
            }
            break;

        case OCS_UP:
            if (activeLevel && (waitTime < _debounceTicks)) {
                _newState(_lastState);
            } else if (waitTime >= _debounceTicks) {
                _nClicks++;
                _newState(OCS_COUNT);
            }
            break;

        case OCS_COUNT:
            if (activeLevel) {
                _newState(OCS_DOWN);
                _startTime = now;
            } else if ((waitTime > _clickTicks) || (_nClicks == _maxClicks)) {
                if (_nClicks == 1) {
                    if (_clickFunc)
                        _clickFunc();
                    if (_paramClickFunc)
                        _paramClickFunc(_clickFuncParam);
                } else if (_nClicks == 2) {
                    if (_doubleClickFunc)
                        _doubleClickFunc();
                    if (_paramDoubleClickFunc)
                        _paramDoubleClickFunc(_doubleClickFuncParam);
                } else {
                    if (_multiClickFunc)
                        _multiClickFunc();
                    if (_paramMultiClickFunc)
                        _paramMultiClickFunc(_multiClickFuncParam);
                }
                reset();
            }
            break;

        case OCS_PRESS:
            if (!activeLevel) {
                _newState(OCS_PRESSEND);
                _startTime = now;
            } else {
                if (_duringLongPressFunc)
                    _duringLongPressFunc();
                if (_paramDuringLongPressFunc)
                    _paramDuringLongPressFunc(_duringLongPressFuncParam);
            }
            break;

        case OCS_PRESSEND:
            if (activeLevel && (waitTime < _debounceTicks)) {
                _newState(_lastState);
            } else if (waitTime >= _debounceTicks) {
                if (_longPressStopFunc)
                    _longPressStopFunc();
                if (_paramLongPressStopFunc)
                    _paramLongPressStopFunc(_longPressStopFuncParam);
                reset();
            }
            break;

        default:
            _newState(OCS_INIT);
            break;
    }
}
