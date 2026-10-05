#include "DisplayManager.h"
#include <math.h>

DisplayManager Display;

DisplayManager::DisplayManager()
    : _tft(TFT_CS, TFT_DC, TFT_RST),
      _currentView(VIEW_EXPLORE),
      _scrollLine(0),
      _maxScrollLines(0),
      _needsFullRedraw(true),
      _sleeping(false),
      _lastStatusBarUpdate(0) {}

void DisplayManager::init() {
    if (TFT_BL >= 0) {
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH);
    }
    _tft.init(240, 320);
    _tft.setRotation(TFT_ROTATION);
    _tft.fillScreen(COLOR_BG);
    _tft.setTextWrap(true);
    _sleeping = false;
}

Adafruit_ST7789& DisplayManager::getTFT() {
    return _tft;
}

void DisplayManager::showBootScreen(const String &status, int progress) {
    _tft.fillScreen(COLOR_BG);
    _tft.setFont(&Roboto_Bold12pt7b);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_ACCENT_AMBER);

    int16_t x1, y1;
    uint16_t w, h;
    _tft.getTextBounds(F("Wik(in)Side"), 0, 0, &x1, &y1, &w, &h);
    _tft.setCursor(65, 70);
    _tft.print(F("Wik(in)Side"));


    // Barra di caricamento
    int barW = 200;
    int barH = 10;
    int barX = 25;
    int barY = 280;

    _tft.drawRoundRect(barX - 2, barY - 2, barW + 4, barH + 4, 3, COLOR_CARD_BORDER);
    int fillW = (barW * progress) / 100;
    if (fillW > 0) {
        _tft.fillRoundRect(barX, barY, fillW, barH, 2, COLOR_ACCENT_GREEN);
    }

    // Stato di caricamento
    _tft.fillRect(20, 165, SCREEN_WIDTH - 40, 24, COLOR_BG);
    _tft.setFont(NULL);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_WHITE);
    int textX = (SCREEN_WIDTH - (status.length() * 6)) / 2;
    if (textX < 20) textX = 20;
    // _tft.setCursor(textX, 172);
    // _tft.print(status);
}

void DisplayManager::switchView(OSView view) {
    if (view != _currentView) {
        _currentView = view;
        _scrollLine = 0;
        _needsFullRedraw = true;
    }
}

void DisplayManager::nextView() {
    int next = ((int)_currentView + 1) % VIEW_COUNT;
    switchView((OSView)next);
}

void DisplayManager::previousView() {
    int prev = ((int)_currentView - 1 + VIEW_COUNT) % VIEW_COUNT;
    switchView((OSView)prev);
}

OSView DisplayManager::getCurrentView() const {
    return _currentView;
}

void DisplayManager::scrollUp() {
    if (_scrollLine > 0) {
        _scrollLine--;
        _needsFullRedraw = true;
    }
}

void DisplayManager::scrollDown() {
    if (_scrollLine < _maxScrollLines) {
        _scrollLine++;
        _needsFullRedraw = true;
    }
}

void DisplayManager::resetScroll() {
    _scrollLine = 0;
    _needsFullRedraw = true;
}

void DisplayManager::drawStatusBar(bool wifiOk, bool gpsOk, int sats) {
    _tft.fillRect(0, 0, SCREEN_WIDTH, 24, COLOR_HEADER);

    // 1. Icona / Indicatore WiFi (a sinistra)
    uint16_t wifiColor = wifiOk ? COLOR_ACCENT_GREEN : COLOR_ACCENT_RED;
    _tft.fillCircle(12, 12, 4, wifiColor);
    _tft.setFont(NULL);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_WHITE);
    _tft.setCursor(22, 8);
    _tft.print(wifiOk ? F("WiFi") : F("No WiFi"));

    // 2. Nome Vista corrente (al centro, con Roboto Regular)
    const char* viewNames[] = { "ESPLORA", "ARTICOLO", "QR CODE", "RADAR", "IMPOSTAZIONI" };
    const char* tabText = viewNames[(int)_currentView];

    _tft.setFont(&Roboto_Regular9pt7b);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_ACCENT_CYAN);

    int16_t x1, y1;
    uint16_t w, h;
    _tft.getTextBounds(tabText, 0, 0, &x1, &y1, &w, &h);
    _tft.setCursor((SCREEN_WIDTH - w) / 2, 17);
    _tft.print(tabText);

    // 3. Indicatore GPS (a destra)
    uint16_t gpsColor = gpsOk ? COLOR_ACCENT_GREEN : COLOR_ACCENT_RED;
    _tft.fillCircle(SCREEN_WIDTH - 60, 12, 4, gpsColor);
    _tft.setFont(NULL);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_WHITE);
    _tft.setCursor(SCREEN_WIDTH - 52, 8);
    if (gpsOk) {
        _tft.print(F("GPS "));
        _tft.print(sats);
    } else {
        _tft.print(F("No GPS"));
    }

    _tft.drawFastHLine(0, 24, SCREEN_WIDTH, COLOR_CARD_BORDER);
}

void DisplayManager::drawExploreView(const MonumentInfo &monument, float lat, float lon) {
    _tft.fillRect(0, 25, SCREEN_WIDTH, SCREEN_HEIGHT - 25, COLOR_BG);

    // Box principale monumento
    // int boxX = 12, boxY = 34, boxW = SCREEN_WIDTH - 48, boxH = 150;
    // _tft.fillRoundRect(boxX, boxY, boxW, boxH, 8, COLOR_CARD_BG);
    // _tft.drawRoundRect(boxX, boxY, boxW, boxH, 8, COLOR_CARD_BORDER);

    if (monument.valid) {
        // Titolo Monumento con Roboto Bold
        _tft.setFont(&Roboto_Bold12pt7b);
        _tft.setTextSize(1);
        _tft.setTextColor(COLOR_ACCENT_CYAN);
        _tft.setCursor(boxX + 12, boxY + 28);
        
        // Troncamento se il titolo è troppo lungo
        String title = monument.title;
        if (title.length() > 20) {
            title = title.substring(0, 18) + "..";
        }
        _tft.print(title);

        // Distanza stimata
        _tft.setFont(&Roboto_Regular9pt7b);
        _tft.setTextColor(COLOR_TEXT_MUTED);
        _tft.setCursor(boxX + 12, boxY + 54);
        _tft.print(F("Distanza:"));

        _tft.setFont(&Roboto_Bold12pt7b);
        _tft.setTextColor(COLOR_ACCENT_AMBER);
        _tft.setCursor(boxX + 12, boxY + 84);
        if (monument.distance >= 1000.0f) {
            _tft.print(monument.distance / 1000.0f, 1);
            _tft.setFont(&Roboto_Regular9pt7b);
            _tft.print(F(" km"));
        } else {
            _tft.print((int)monument.distance);
            _tft.setFont(&Roboto_Regular9pt7b);
            _tft.print(F(" m"));
        }

        // Anteprima testo
        _tft.setFont(NULL);
        _tft.setTextSize(1);
        _tft.setTextColor(COLOR_TEXT_WHITE);
        _tft.setCursor(boxX + 12, boxY + 104);
        String preview = monument.extract;
        if (preview.length() > 140) {
            preview = preview.substring(0, 137) + "...";
        }
        _tft.print(preview);

    } else {
        _tft.setFont(&Roboto_Bold12pt7b);
        _tft.setTextSize(1);
        _tft.setTextColor(COLOR_TEXT_MUTED);
        _tft.setCursor(boxX + 25, boxY + 60);
        _tft.print(F("Ricerca"));
        _tft.setCursor(boxX + 25, boxY + 90);
        _tft.print(F("monumenti..."));
    }


    // Barra istruzioni footer
    _tft.setTextColor(COLOR_ACCENT_CYAN);
    _tft.setCursor(15, 300);
    _tft.print(F("Muoviti con l'analogico"));
}

void DisplayManager::drawArticleView(const MonumentInfo &monument) {
    _tft.fillRect(0, 25, SCREEN_WIDTH - 50, SCREEN_HEIGHT - 25, COLOR_BG);

    if (!monument.valid || monument.extract.length() == 0) {
        _tft.setFont(&Roboto_Bold12pt7b);
        _tft.setTextSize(1);
        _tft.setTextColor(COLOR_TEXT_MUTED);
        _tft.setCursor(20, 80);
        _tft.print(F("Nessun articolo"));

        _tft.setFont(&Roboto_Regular9pt7b);
        _tft.setCursor(20, 115);
        _tft.print(F("Trova un monumento"));
        _tft.setCursor(20, 140);
        _tft.print(F("nella vista Esplora."));
        return;
    }

    // Titolo in Roboto Bold
    _tft.setFont(&Roboto_Bold12pt7b);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_ACCENT_CYAN);
    _tft.setCursor(12, 48);
    String title = monument.title;
    if (title.length() > 20) title = title.substring(0, 18) + "..";
    _tft.print(title);
    _tft.drawFastHLine(12, 56, SCREEN_WIDTH - 24, COLOR_CARD_BORDER);

    // Disegna testo a scorrimento in font standard (compatto e scorrevole)
    _tft.setFont(NULL);
    wrapAndDrawText(monument.extract, 12, 64, SCREEN_WIDTH - 30, 146, _scrollLine);

    // Footer
    _tft.setFont(NULL);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_MUTED);
    _tft.setCursor(12, 222);
    _tft.print(F("[Su/Giu] Scorri   |   [OK] Mostra QR Code"));
}

void DisplayManager::wrapAndDrawText(const String &text, int startX, int startY, int maxW, int maxH, int lineScroll) {
    _tft.setFont(NULL);
    int maxCharsPerLine = maxW / 6;
    int maxDisplayLines = maxH / 12;

    int currentLine = 0;
    int displayedLines = 0;
    int textLen = text.length();
    int idx = 0;

    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_WHITE);

    while (idx < textLen) {
        // Trova limite riga
        int endIdx = idx + maxCharsPerLine;
        if (endIdx >= textLen) {
            endIdx = textLen;
        } else {
            // Spezza allo spazio
            int spaceIdx = text.lastIndexOf(' ', endIdx);
            if (spaceIdx > idx) {
                endIdx = spaceIdx;
            }
        }

        if (currentLine >= lineScroll && displayedLines < maxDisplayLines) {
            _tft.setCursor(startX, startY + (displayedLines * 12));
            _tft.print(text.substring(idx, endIdx));
            displayedLines++;
        }

        currentLine++;
        idx = endIdx;
        while (idx < textLen && text.charAt(idx) == ' ') idx++;
    }

    _maxScrollLines = (currentLine > maxDisplayLines) ? (currentLine - maxDisplayLines) : 0;

    // Disegna barra di scorrimento (Scrollbar)
    if (_maxScrollLines > 0) {
        int scrollBarH = maxH;
        int thumbH = max(10, scrollBarH * maxDisplayLines / currentLine);
        int thumbY = startY + (_scrollLine * (scrollBarH - thumbH) / _maxScrollLines);
        _tft.fillRect(SCREEN_WIDTH - 10, startY, 4, scrollBarH, COLOR_CARD_BG);
        _tft.fillRect(SCREEN_WIDTH - 10, thumbY, 4, thumbH, COLOR_ACCENT_CYAN);
    }
}

void DisplayManager::drawQRView(const MonumentInfo &monument) {
    _tft.fillRect(0, 25, SCREEN_WIDTH, SCREEN_HEIGHT - 25, COLOR_BG);

    String urlToEncode = monument.valid ? monument.url : "https://it.wikipedia.org";

    // Genera QR 
    QR.generate(urlToEncode.c_str());
    QR.drawToDisplay(_tft, 60, 112, 136, 0x0000, 0xFFFF);


    _tft.setFont(&Roboto_Regular9pt7b);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_WHITE);

    int16_t x1, y1;
    uint16_t w, h;
    _tft.getTextBounds(F("Inquadra con fotocamera"), 0, 0, &x1, &y1, &w, &h);
    _tft.setCursor(10, 202);
    _tft.print(F("Inquadra con fotocamera"));

    _tft.setFont(NULL);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_MUTED);
    String shortUrl = urlToEncode;
    if (shortUrl.length() > 45) shortUrl = shortUrl.substring(0, 42) + "...";
    int urlX = (SCREEN_WIDTH - (shortUrl.length() * 6)) / 2;
    if (urlX < 10) urlX = 10;
    _tft.setCursor(10, 218);
    _tft.print(shortUrl);
}

void DisplayManager::drawRadarView(const MonumentInfo &monument, float userLat, float userLon) {
    _tft.fillRect(0, 25, SCREEN_WIDTH, SCREEN_HEIGHT - 25, COLOR_BG);

    int centerX = 110;
    int centerY = 132;
    int radius = 75;

    // Cerchi concentrici radar
    _tft.drawCircle(centerX, centerY, radius, COLOR_CARD_BORDER);
    _tft.drawCircle(centerX, centerY, radius * 2 / 3, COLOR_CARD_BORDER);
    _tft.drawCircle(centerX, centerY, radius / 3, COLOR_CARD_BORDER);

    // Assi a croce
    _tft.drawFastHLine(centerX - radius - 5, centerY, (radius + 5) * 2, COLOR_CARD_BORDER);
    _tft.drawFastVLine(centerX, centerY - radius - 5, (radius + 5) * 2, COLOR_CARD_BORDER);

    // Lettere cardinali
    _tft.setFont(NULL);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_ACCENT_CYAN);
    _tft.setCursor(centerX - 3, centerY - radius - 15);
    _tft.print(F("N"));

    // Posizione utente al centro
    _tft.fillCircle(centerX, centerY, 3, COLOR_ACCENT_CYAN);

    // Se c'è un monumento valido, calcola angolo e disegna il blip
    if (monument.valid && monument.lat != 0.0f) {
        float dLat = (monument.lat - userLat) * 111320.0f;
        float dLon = (monument.lon - userLon) * 40075000.0f * cos(userLat * 0.0174532925f) / 360.0f;

        float angle = atan2(dLon, dLat); // Radianti da Nord in senso orario
        float blipDistRatio = min(1.0f, monument.distance / 500.0f);
        int blipDistPix = (int)(blipDistRatio * (radius - 8));

        int blipX = centerX + (int)(sin(angle) * blipDistPix);
        int blipY = centerY - (int)(cos(angle) * blipDistPix);

        // Blip del monumento
        _tft.fillCircle(blipX, blipY, 5, COLOR_ACCENT_AMBER);
        _tft.drawCircle(blipX, blipY, 7, COLOR_ACCENT_RED);
    }

    // Dati monumento a lato (destra)
    int infoX = 205;
    _tft.setFont(&Roboto_Regular9pt7b);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_ACCENT_AMBER);
    _tft.setCursor(infoX - 100, 55);
    _tft.print(F("BERSAGLIO:"));

    _tft.setTextColor(COLOR_TEXT_WHITE);
    _tft.setCursor(infoX, 78);
    if (monument.valid) {
        String t = monument.title;
        if (t.length() > 11) t = t.substring(0, 9) + "..";
        _tft.print(t);

        _tft.setTextColor(COLOR_TEXT_MUTED);
        _tft.setCursor(infoX, 105);
        _tft.print(F("DISTANZA:"));

        _tft.setFont(&Roboto_Bold12pt7b);
        _tft.setTextColor(COLOR_ACCENT_GREEN);
        _tft.setCursor(infoX, 130);
        _tft.print((int)monument.distance);
        _tft.setFont(&Roboto_Regular9pt7b);
        _tft.print(F("m"));
    } else {
        _tft.print(F("Nessuno"));
    }

    _tft.setFont(NULL);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_MUTED);
    _tft.setCursor(15, 222);
    _tft.print(F("Radar portata max: 500m"));
}

void DisplayManager::drawSettingsView(bool wifiOk) {
    _tft.fillRect(0, 25, SCREEN_WIDTH, SCREEN_HEIGHT - 25, COLOR_BG);

    int startY = 48;
    _tft.setFont(&Roboto_Bold12pt7b);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_ACCENT_CYAN);
    _tft.setCursor(20, startY);
    _tft.print(F("Impostazioni OS"));
    _tft.drawFastHLine(20, startY + 8, SCREEN_WIDTH - 40, COLOR_CARD_BORDER);

    _tft.setFont(&Roboto_Regular9pt7b);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_WHITE);

    _tft.setCursor(20, startY + 34);
    _tft.print(F("Raggio Ricerca:   "));
    _tft.setTextColor(COLOR_ACCENT_AMBER);
    _tft.print(WIKI_DEFAULT_RADIUS);
    _tft.print(F(" m"));

    _tft.setTextColor(COLOR_TEXT_WHITE);
    _tft.setCursor(20, startY + 58);
    _tft.print(F("Feedback Audio:   ATTIVO"));

    _tft.setCursor(20, startY + 82);
    _tft.print(F("Feedback Aptico:  ATTIVO"));

    _tft.setCursor(20, startY + 106);
    _tft.print(F("Rete Wi-Fi:       "));
    _tft.setTextColor(wifiOk ? COLOR_ACCENT_GREEN : COLOR_ACCENT_RED);
    _tft.print(wifiOk ? F("CONNESSO") : F("DISCONNESSO"));

    _tft.setFont(NULL);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_MUTED);
    _tft.setCursor(20, startY + 130);
    _tft.print(F("Modalita Test: Digita \"TEST\" sulla seriale"));

    _tft.setCursor(20, 218);
    _tft.setTextColor(COLOR_ACCENT_CYAN);
    _tft.print(F("[◄/►] Cambia Vista"));
}

void DisplayManager::sleep() {
    if (!_sleeping) {
        _sleeping = true;
        _tft.fillScreen(COLOR_BG);
        _tft.enableDisplay(false);
        _tft.enableSleep(true);
        if (TFT_BL >= 0) {
            digitalWrite(TFT_BL, LOW);
        }
    }
}

void DisplayManager::wakeup() {
    if (_sleeping) {
        _sleeping = false;
        _tft.enableSleep(false);
        delay(15);
        _tft.enableDisplay(true);
        if (TFT_BL >= 0) {
            digitalWrite(TFT_BL, HIGH);
        }
        _needsFullRedraw = true;
        _lastStatusBarUpdate = 0;
    }
}

bool DisplayManager::isSleeping() const {
    return _sleeping;
}

void DisplayManager::update(const MonumentInfo &monument, bool wifiOk, bool gpsOk, int sats, float lat, float lon) {
    if (_sleeping) {
        return;
    }

    // Aggiorna status bar ogni secondo o se serve ridisegno completo
    if (_needsFullRedraw || millis() - _lastStatusBarUpdate > 1000) {
        drawStatusBar(wifiOk, gpsOk, sats);
        _lastStatusBarUpdate = millis();
    }

    if (_needsFullRedraw) {
        switch (_currentView) {
            case VIEW_EXPLORE:
                drawExploreView(monument, lat, lon);
                break;
            case VIEW_ARTICLE:
                drawArticleView(monument);
                break;
            case VIEW_QR:
                drawQRView(monument);
                break;
            case VIEW_RADAR:
                drawRadarView(monument, lat, lon);
                break;
            case VIEW_SETTINGS:
                drawSettingsView(wifiOk);
                break;
            default:
                break;
        }
        _needsFullRedraw = false;
    }
}

void DisplayManager::showTestModeScreen(const String &testName, const String &info1, const String &info2) {
    _tft.fillScreen(0x0000); // Nero assoluto

    // Banner superiore di test
    _tft.fillRect(0, 0, SCREEN_WIDTH, 32, COLOR_ACCENT_AMBER);
    _tft.setFont(&Roboto_Bold12pt7b);
    _tft.setTextSize(1);
    _tft.setTextColor(0x0000);
    _tft.setCursor(15, 23);
    _tft.print(F("DIAGNOSTICA SERIALE"));

    // Nome del test
    _tft.setFont(&Roboto_Bold12pt7b);
    _tft.setTextColor(COLOR_ACCENT_CYAN);
    _tft.setCursor(15, 58);
    _tft.print(testName);
    _tft.drawFastHLine(15, 68, SCREEN_WIDTH - 30, COLOR_CARD_BORDER);

    // Linee descrittive
    _tft.setFont(&Roboto_Regular9pt7b);
    _tft.setTextColor(COLOR_TEXT_WHITE);
    _tft.setCursor(15, 92);
    _tft.print(info1);

    if (info2.length() > 0) {
        _tft.setCursor(15, 114);
        _tft.print(info2);
    }

    _tft.setFont(NULL);
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_ACCENT_AMBER);
    _tft.setCursor(15, 215);
    _tft.print(F("Invia 'X' o premi pulsante per uscire"));
}

void DisplayManager::renderLiveJoyCrosshair(int rawX, int rawY, bool pressed) {
    _tft.setFont(NULL);
    int areaX = 15, areaY = 120, areaW = 290, areaH = 80;
    _tft.fillRect(areaX, areaY, areaW, areaH, COLOR_CARD_BG);
    _tft.drawRect(areaX, areaY, areaW, areaH, COLOR_CARD_BORDER);

    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_WHITE);
    _tft.setCursor(areaX + 10, areaY + 15);
    _tft.print(F("VRX: "));
    _tft.print(rawX);
    _tft.setCursor(areaX + 10, areaY + 35);
    _tft.print(F("VRY: "));
    _tft.print(rawY);

    _tft.setCursor(areaX + 10, areaY + 55);
    _tft.print(F("Pulsante SW: "));
    if (pressed) {
        _tft.setTextColor(COLOR_ACCENT_GREEN);
        _tft.print(F("PREMUTO / CLICK"));
    } else {
        _tft.setTextColor(COLOR_TEXT_MUTED);
        _tft.print(F("RILASCIATO"));
    }

    // Mini box con mirino grafico a destra
    int boxSize = 60;
    int boxStartX = areaX + 210;
    int boxStartY = areaY + 10;
    _tft.drawRect(boxStartX, boxStartY, boxSize, boxSize, COLOR_CARD_BORDER);
    _tft.drawFastHLine(boxStartX, boxStartY + (boxSize / 2), boxSize, COLOR_CARD_BORDER);
    _tft.drawFastVLine(boxStartX + (boxSize / 2), boxStartY, boxSize, COLOR_CARD_BORDER);

    int crossX = map(rawX, 0, 4095, boxStartX + 2, boxStartX + boxSize - 3);
    int crossY = map(rawY, 0, 4095, boxStartY + 2, boxStartY + boxSize - 3);
    _tft.fillCircle(crossX, crossY, 3, pressed ? COLOR_ACCENT_GREEN : COLOR_ACCENT_CYAN);
}
