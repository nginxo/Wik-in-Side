#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "config.h"
#include "WikiClient.h"
#include "QRManager.h"

enum OSView {
    VIEW_EXPLORE = 0,
    VIEW_ARTICLE,
    VIEW_QR,
    VIEW_RADAR,
    VIEW_SETTINGS,
    VIEW_COUNT
};

class DisplayManager {
public:
    DisplayManager();

    void init();
    void showBootScreen(const String &status, int progress);

    void switchView(OSView view);
    void nextView();
    void previousView();
    OSView getCurrentView() const;

    void scrollUp();
    void scrollDown();
    void resetScroll();

    // Ridisegna l'interfaccia corrente
    void update(const MonumentInfo &monument, bool wifiOk, bool gpsOk, int sats, float lat, float lon);

    // Schermata per la modalità Test da Seriale
    void showTestModeScreen(const String &testName, const String &info1, const String &info2 = "");
    void renderLiveJoyCrosshair(int rawX, int rawY, bool pressed);

    // Gestione risparmio energetico e spegnimento schermo
    void sleep();
    void wakeup();
    bool isSleeping() const;

    Adafruit_ST7789& getTFT();

private:
    Adafruit_ST7789 _tft;
    OSView _currentView;
    int _scrollLine;
    int _maxScrollLines;
    bool _needsFullRedraw;
    bool _sleeping;
    uint32_t _lastStatusBarUpdate;

    void drawStatusBar(bool wifiOk, bool gpsOk, int sats);
    void drawExploreView(const MonumentInfo &monument, float lat, float lon);
    void drawArticleView(const MonumentInfo &monument);
    void drawQRView(const MonumentInfo &monument);
    void drawRadarView(const MonumentInfo &monument, float userLat, float userLon);
    void drawSettingsView(bool wifiOk);

    void wrapAndDrawText(const String &text, int startX, int startY, int maxW, int maxH, int lineScroll);
};

extern DisplayManager Display;
