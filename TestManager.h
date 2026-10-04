#pragma once

#include <Arduino.h>
#include "config.h"

enum TestSubMode {
    TEST_MODE_MENU = 0,
    TEST_MODE_DISPLAY_JOYSTICK, // Riferimento: wikipedia.ino
    TEST_MODE_AUDIO_VIBRO,      // Riferimento: audiovibration.c
    TEST_MODE_QR_GEN,           // Riferimento: qrGen.c
    TEST_MODE_GPS,              // Riferimento: wikishootme.cpp (GPS)
    TEST_MODE_WIFI_WIKI,        // Riferimento: wikishootme.cpp (WiFi/API)
    TEST_MODE_SELF_TEST         // Test completo automatico
};

class TestManager {
public:
    TestManager();

    void init();
    bool isInTestMode() const;
    void enterTestMode();
    void exitTestMode();

    // Controlla se la seriale ha ricevuto il comando "TEST"
    bool checkSerialTrigger();

    // Loop principale quando siamo in modalità test
    void update();

private:
    bool _active;
    TestSubMode _subMode;
    String _serialRxBuffer;
    uint32_t _lastLiveUpdate;

    void printMainMenu();
    void runDisplayJoystickTest();
    void runAudioVibroTest();
    void runQRGenTest();
    void runGPSTest();
    void runWiFiWikiTest();
    void runSelfTest();

    void handleSerialMenuInput(char c);
};

extern TestManager Diagnostics;
