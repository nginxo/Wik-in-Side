#pragma once

#include <Arduino.h>
#include "config.h"

enum JoyDirection {
    JOY_DIR_NONE = 0,
    JOY_DIR_UP,
    JOY_DIR_DOWN,
    JOY_DIR_LEFT,
    JOY_DIR_RIGHT
};

class JoystickManager {
public:
    JoystickManager();

    void init();
    void update();

    // Letture grezze (utili per il Test Mode)
    int getRawX() const;
    int getRawY() const;
    bool isButtonPressed() const;

    // Letture calibrate e direzionali per l'OS
    JoyDirection getDirection() const;
    bool justMoved(JoyDirection dir);
    bool buttonJustPressed();
    bool buttonJustReleased();
    bool buttonHeld(uint32_t durationMs = 800);
    bool buttonHeldTrigger(uint32_t durationMs = 3000);
    bool buttonClicked();
    uint32_t getButtonPressDuration() const;
    bool anyMovement() const;

    // Ripetizione movimento tenuto premuto (ideale per scroll rapido testi)
    bool isHoldRepeating(JoyDirection dir, uint32_t initialDelayMs = 400, uint32_t repeatRateMs = 120);

private:
    int _rawX;
    int _rawY;
    int _centerX;
    int _centerY;
    bool _btnState;
    bool _lastBtnState;
    bool _btnHeldTriggered;
    uint32_t _btnPressStartTime;

    JoyDirection _currentDir;
    JoyDirection _lastDir;
    uint32_t _dirHoldStartTime;
    uint32_t _lastRepeatTime;
};

extern JoystickManager Joystick;
