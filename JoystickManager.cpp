#include "JoystickManager.h"

JoystickManager Joystick;

JoystickManager::JoystickManager()
    : _rawX(2048), _rawY(2048), _centerX(2048), _centerY(2048),
      _btnState(false), _lastBtnState(false), _btnHeldTriggered(false),
      _btnPressStartTime(0),
      _currentDir(JOY_DIR_NONE), _lastDir(JOY_DIR_NONE),
      _dirHoldStartTime(0), _lastRepeatTime(0) {}

void JoystickManager::init() {
    pinMode(JOY_SW_PIN, INPUT_PULLUP);
    
    // Calibrazione automatica del centro nei primi cicli
    long sumX = 0, sumY = 0;
    const int samples = 20;
    for (int i = 0; i < samples; i++) {
        sumX += analogRead(JOY_VRX_PIN);
        sumY += analogRead(JOY_VRY_PIN);
        delay(5);
    }
    _centerX = sumX / samples;
    _centerY = sumY / samples;
}

void JoystickManager::update() {
    _lastBtnState = _btnState;
    _lastDir = _currentDir;

    // Lettura assi analogici
    _rawX = analogRead(JOY_VRX_PIN);
    _rawY = analogRead(JOY_VRY_PIN);

    // Lettura pulsante: solitamente con INPUT_PULLUP, LOW = premuto
    // Supportiamo sia pulsante attivo basso (standard con pullup) che attivo alto
    int pinVal = digitalRead(JOY_SW_PIN);
    _btnState = (pinVal == LOW);

    if (_btnState && !_lastBtnState) {
        _btnPressStartTime = millis();
        _btnHeldTriggered = false;
    }

    // Calcolo scostamento rispetto al centro
    int dx = _rawX - _centerX;
    int dy = _rawY - _centerY;

    if (JOY_INVERT_X) dx = -dx;
    if (JOY_INVERT_Y) dy = -dy;

    JoyDirection newDir = JOY_DIR_NONE;

    if (abs(dx) > JOY_DEADZONE || abs(dy) > JOY_DEADZONE) {
        if (abs(dx) > abs(dy)) {
            newDir = (dx > 0) ? JOY_DIR_RIGHT : JOY_DIR_LEFT;
        } else {
            // Nota standard assi: solitamente spostandosi in alto la lettura scende o sale a seconda del montaggio
            newDir = (dy > 0) ? JOY_DIR_DOWN : JOY_DIR_UP;
        }
    }

    if (newDir != _currentDir) {
        _currentDir = newDir;
        _dirHoldStartTime = millis();
        _lastRepeatTime = millis();
    }
}

int JoystickManager::getRawX() const {
    return _rawX;
}

int JoystickManager::getRawY() const {
    return _rawY;
}

bool JoystickManager::isButtonPressed() const {
    return _btnState;
}

JoyDirection JoystickManager::getDirection() const {
    return _currentDir;
}

bool JoystickManager::justMoved(JoyDirection dir) {
    return (_currentDir == dir && _lastDir != dir);
}

bool JoystickManager::buttonJustPressed() {
    return (_btnState && !_lastBtnState);
}

bool JoystickManager::buttonJustReleased() {
    return (!_btnState && _lastBtnState);
}

bool JoystickManager::buttonHeld(uint32_t durationMs) {
    return (_btnState && (millis() - _btnPressStartTime >= durationMs));
}

bool JoystickManager::buttonHeldTrigger(uint32_t durationMs) {
    if (_btnState && !_btnHeldTriggered && (millis() - _btnPressStartTime >= durationMs)) {
        _btnHeldTriggered = true;
        return true;
    }
    return false;
}

bool JoystickManager::buttonClicked() {
    if (!_btnState && _lastBtnState) {
        if (!_btnHeldTriggered && (millis() - _btnPressStartTime < 2500)) {
            return true;
        }
    }
    return false;
}

uint32_t JoystickManager::getButtonPressDuration() const {
    if (_btnState) {
        return millis() - _btnPressStartTime;
    }
    return 0;
}

bool JoystickManager::anyMovement() const {
    if (_currentDir != JOY_DIR_NONE) return true;
    int dx = abs(_rawX - _centerX);
    int dy = abs(_rawY - _centerY);
    return (dx > JOY_DEADZONE || dy > JOY_DEADZONE);
}

bool JoystickManager::isHoldRepeating(JoyDirection dir, uint32_t initialDelayMs, uint32_t repeatRateMs) {
    if (_currentDir != dir) return false;

    uint32_t now = millis();
    if (now - _dirHoldStartTime < initialDelayMs) {
        return justMoved(dir);
    }

    if (now - _lastRepeatTime >= repeatRateMs) {
        _lastRepeatTime = now;
        return true;
    }

    return false;
}
