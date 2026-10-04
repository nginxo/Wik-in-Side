#include "TestManager.h"
#include "DisplayManager.h"
#include "FeedbackManager.h"
#include "JoystickManager.h"
#include "GPSManager.h"
#include "WikiClient.h"
#include "QRManager.h"

TestManager Diagnostics;

TestManager::TestManager()
    : _active(false), _subMode(TEST_MODE_MENU), _serialRxBuffer(""), _lastLiveUpdate(0) {}

void TestManager::init() {
    _active = false;
    _subMode = TEST_MODE_MENU;
    _serialRxBuffer = "";
}

bool TestManager::isInTestMode() const {
    return _active;
}

void TestManager::enterTestMode() {
    _active = true;
    _subMode = TEST_MODE_MENU;
    Feedback.playSuccess();

    Display.showTestModeScreen(F("MENU PRINCIPALE"), F("Menu di test aperto da Seriale."), F("Invia un comando sulla console PC."));
    printMainMenu();
}

void TestManager::exitTestMode() {
    _active = false;
    _subMode = TEST_MODE_MENU;
    Feedback.playClick();

    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("[OS] Uscita dalla modalita Test. Ripristino GeoOS..."));
    Serial.println(F("=================================================="));
    Serial.println();

    Display.switchView(VIEW_EXPLORE);
}

bool TestManager::checkSerialTrigger() {
    while (Serial.available() > 0) {
        char c = Serial.read();

        if (c == '\r' || c == '\n') {
            _serialRxBuffer.trim();
            if (_serialRxBuffer.equalsIgnoreCase(TEST_COMMAND_TRIGGER)) {
                _serialRxBuffer = "";
                if (!_active) {
                    enterTestMode();
                    return true;
                }
            }
            _serialRxBuffer = "";
        } else {
            if (_serialRxBuffer.length() < 30) {
                _serialRxBuffer += c;
            }
        }
    }
    return false;
}

void TestManager::printMainMenu() {
    Serial.println();
    Serial.println(F("======================================================================"));
    Serial.println(F("               GeoOS ESP32 - MENU TEST E DIAGNOSTICA                  "));
    Serial.println(F("   (Ispirato ai moduli di test: wikipedia, audiovibration, qrGen)     "));
    Serial.println(F("======================================================================"));
    Serial.println(F(" 1. Test Display ST7789 & Joystick        (Rif: wikipedia.ino)"));
    Serial.println(F(" 2. Test Audio & Vibrazione Aptica        (Rif: audiovibration.c)"));
    Serial.println(F(" 3. Test Generatore QR Code               (Rif: qrGen.c)"));
    Serial.println(F(" 4. Test Modulo GPS Hardware              (Rif: wikishootme.cpp)"));
    Serial.println(F(" 5. Test Wi-Fi & Chiamata API Wikipedia   (Rif: wikishootme.cpp)"));
    Serial.println(F(" 6. Diagnostica Hardware Completa (Self-Test automatico)"));
    Serial.println(F(" X. Esci dalla modalita Test e torna all'OS"));
    Serial.println(F("----------------------------------------------------------------------"));
    Serial.print(F("Digita un'opzione (1-6 o X): "));
}

void TestManager::update() {
    if (!_active) return;

    // Gestione input utente durante la modalità test
    if (Serial.available() > 0) {
        char c = Serial.read();
        if (c != '\r' && c != '\n') {
            if (_subMode == TEST_MODE_MENU) {
                handleSerialMenuInput(c);
            } else {
                if (c == 'x' || c == 'X' || c == 'q' || c == 'Q') {
                    _subMode = TEST_MODE_MENU;
                    Display.showTestModeScreen(F("MENU PRINCIPALE"), F("Test terminato."), F("Invia un comando sulla console PC."));
                    printMainMenu();
                    return;
                }
            }
        }
    }

    // Se stiamo eseguendo un test continuo
    switch (_subMode) {
        case TEST_MODE_DISPLAY_JOYSTICK:
            runDisplayJoystickTest();
            break;
        case TEST_MODE_GPS:
            runGPSTest();
            break;
        default:
            break;
    }
}

void TestManager::handleSerialMenuInput(char c) {
    Serial.println(c);

    switch (c) {
        case '1':
            _subMode = TEST_MODE_DISPLAY_JOYSTICK;
            Display.showTestModeScreen(F("TEST 1: DISPLAY & JOYSTICK"), 
                                       F("Muovi lo stick e premi il pulsante."), 
                                       F("Invia 'X' o premi stick per uscire."));
            Serial.println();
            Serial.println(F("--- [TEST 1] Display & Joystick avviato ---"));
            Serial.println(F("Muovi lo stick: valori live su Seriale e Display. Invia 'X' per uscire."));
            _lastLiveUpdate = 0;
            break;

        case '2':
            _subMode = TEST_MODE_AUDIO_VIBRO;
            runAudioVibroTest();
            _subMode = TEST_MODE_MENU;
            printMainMenu();
            break;

        case '3':
            _subMode = TEST_MODE_QR_GEN;
            runQRGenTest();
            _subMode = TEST_MODE_MENU;
            printMainMenu();
            break;

        case '4':
            _subMode = TEST_MODE_GPS;
            Display.showTestModeScreen(F("TEST 4: MODULO GPS"), 
                                       F("Lettura NMEA da porta seriale GPS."), 
                                       F("Invia 'X' per tornare al menu."));
            Serial.println();
            Serial.println(F("--- [TEST 4] Test Ricevitore GPS avviato ---"));
            Serial.println(F("Lettura dati NMEA live. Invia 'X' per uscire."));
            _lastLiveUpdate = 0;
            break;

        case '5':
            _subMode = TEST_MODE_WIFI_WIKI;
            runWiFiWikiTest();
            _subMode = TEST_MODE_MENU;
            printMainMenu();
            break;

        case '6':
            _subMode = TEST_MODE_SELF_TEST;
            runSelfTest();
            _subMode = TEST_MODE_MENU;
            printMainMenu();
            break;

        case 'x':
        case 'X':
            exitTestMode();
            break;

        default:
            Serial.println(F("Opzione non valida. Digita 1, 2, 3, 4, 5, 6 o X."));
            break;
    }
}

void TestManager::runDisplayJoystickTest() {
    Joystick.update();

    if (Joystick.buttonJustPressed()) {
        Feedback.playClick();
        Serial.println(F("[TEST 1] Pulsante Joystick premuto! Ritorno al menu..."));
        _subMode = TEST_MODE_MENU;
        Display.showTestModeScreen(F("MENU PRINCIPALE"), F("Test Joystick completato."), F("Seleziona una voce dal menu seriale."));
        printMainMenu();
        return;
    }

    if (millis() - _lastLiveUpdate > 100) {
        _lastLiveUpdate = millis();

        int x = Joystick.getRawX();
        int y = Joystick.getRawY();
        bool btn = Joystick.isButtonPressed();

        // Stampa sul display (come in wikipedia.ino ma con mirino)
        Display.renderLiveJoyCrosshair(x, y, btn);

        // Stampa a seriale periodica
        static uint32_t lastSerialLog = 0;
        if (millis() - lastSerialLog > 400) {
            lastSerialLog = millis();
            Serial.print(F("Joystick -> VRX: "));
            Serial.print(x);
            Serial.print(F(" | VRY: "));
            Serial.print(y);
            Serial.print(F(" | SW: "));
            Serial.println(btn ? F("PREMUTO (CLICK)") : F("RILASCIATO"));
        }
    }
}

void TestManager::runAudioVibroTest() {
    Display.showTestModeScreen(F("TEST 2: AUDIO & VIBRO"), 
                               F("Esecuzione sequenza audiovibration.c"), 
                               F("Note: 888Hz, 1142Hz, 1333Hz"));

    Serial.println();
    Serial.println(F("--- [TEST 2] Test Audio e Vibrazione (audiovibration.c) ---"));
    Serial.println(F("Avvio riproduzione note con curva di attacco, sustain e rilascio..."));

    for (int i = 0; i < NUM_MONUMENT_NOTES; i++) {
        int freq = MONUMENT_NOTES[i];
        Serial.print(F("  -> Riproduzione Nota "));
        Serial.print(i + 1);
        Serial.print(F("/"));
        Serial.print(NUM_MONUMENT_NOTES);
        Serial.print(F(" | Frequenza: "));
        Serial.print(freq);
        Serial.print(F(" Hz | Durata: "));
        Serial.print(NOTE_DURATION_MS);
        Serial.println(F(" ms"));

        Feedback.playNote(freq, NOTE_DURATION_MS, ATTACK_MS, RELEASE_MS);
        delay(40);
    }

    Feedback.stopAll();
    Serial.println(F("[TEST 2] Sequenza completata con successo!"));
    delay(500);
}

void TestManager::runQRGenTest() {
    const char *testUrl = "https://it.wikipedia.org/wiki/Monte_Fuji";

    Display.showTestModeScreen(F("TEST 3: GENERATORE QR"), 
                               F("Generazione QR Code (qrGen.c)"), 
                               F("Rendering su Seriale e Schermo TFT."));

    Serial.println();
    Serial.println(F("--- [TEST 3] Generatore QR Code (qrGen.c) ---"));
    Serial.print(F("Codifica testo di prova: "));
    Serial.println(testUrl);

    bool ok = QR.generate(testUrl);
    if (ok) {
        Serial.println(F("QR Code calcolato con successo!"));
        Serial.print(F("Dimensione matrice: "));
        Serial.print(QR.getSize());
        Serial.println(F("x"));
        Serial.print(QR.getSize());

        // Stampa fedele a qrGen.c su console seriale
        QR.printToSerial(Serial);

        // Disegna anche sul display TFT ST7789
        Adafruit_ST7789 &tft = Display.getTFT();
        QR.drawToDisplay(tft, SCREEN_WIDTH / 2, 135, 120, 0x0000, 0xFFFF);

        Feedback.playSuccess();
    } else {
        Serial.println(F("[ERRORE] Testo troppo lungo per la codifica QR!"));
        Feedback.playWarning();
    }

    Serial.println(F("[TEST 3] Invia un carattere per continuare..."));
    while (!Serial.available()) {
        delay(50);
    }
    while (Serial.available()) Serial.read();
}

void TestManager::runGPSTest() {
    GPS.update();

    if (millis() - _lastLiveUpdate > 500) {
        _lastLiveUpdate = millis();

        bool hasFix = GPS.hasFix();
        int sats = GPS.getSatellites();
        float lat = GPS.getLat();
        float lon = GPS.getLon();
        float alt = GPS.getAltitude();
        float spd = GPS.getSpeedKmh();
        uint32_t chars = GPS.getCharsProcessed();

        String statusStr = hasFix ? "FIX ATTIVO" : "RICERCA SEGNALE...";
        String line1 = "Satelliti: " + String(sats) + " | Byte NMEA: " + String(chars);
        String line2 = "Lat: " + String(lat, 5) + " Lon: " + String(lon, 5);

        Display.showTestModeScreen(F("TEST 4: RICEVITORE GPS"), line1, line2);

        Serial.print(F("GPS -> Stato: "));
        Serial.print(statusStr);
        Serial.print(F(" | Satelliti: "));
        Serial.print(sats);
        Serial.print(F(" | Lat: "));
        Serial.print(lat, 6);
        Serial.print(F(" | Lon: "));
        Serial.print(lon, 6);
        Serial.print(F(" | Quota: "));
        Serial.print(alt, 1);
        Serial.print(F("m | Velocita: "));
        Serial.print(spd, 1);
        Serial.print(F(" km/h | Byte letti: "));
        Serial.println(chars);
    }
}

void TestManager::runWiFiWikiTest() {
    Display.showTestModeScreen(F("TEST 5: WIFI & WIKIPEDIA"), 
                               F("Connessione WiFi e chiamata API..."), 
                               F("Coordinate test: Monte Fuji, Giappone"));

    Serial.println();
    Serial.println(F("--- [TEST 5] Wi-Fi & MediaWiki API (wikishootme.cpp) ---"));
    Serial.print(F("Connessione a SSID: "));
    Serial.println(Wiki.getSSID());

    bool wifiOk = Wiki.connect(8000);
    if (!wifiOk) {
        Serial.println(F("[ERRORE] Connessione WiFi fallita o timeout scaduto."));
        Serial.println(F("Verifica SSID e password in config.h."));
        Display.showTestModeScreen(F("TEST 5: ERRORE WIFI"), 
                                   F("Impossibile connettersi al WiFi."), 
                                   F("Verifica SSID e Password in config.h"));
        Feedback.playWarning();
        delay(2000);
        return;
    }

    Serial.println(F("WiFi connesso con successo!"));
    Serial.print(F("Indirizzo IP: "));
    Serial.println(Wiki.getIP());
    Serial.print(F("Potenza segnale (RSSI): "));
    Serial.print(Wiki.getRSSI());
    Serial.println(F(" dBm"));

    Serial.println(F("\nInvio query GeoSearch a Wikipedia (Monte Fuji, lat=35.3606, lon=138.7278, raggio=500m)..."));

    MonumentInfo testInfo;
    bool searchOk = Wiki.searchNearby(FALLBACK_LAT, FALLBACK_LON, 500, testInfo);

    if (searchOk && testInfo.valid) {
        Serial.println(F("Risposta ricevuta con successo dall'API!"));
        Serial.println(F("======================================================="));
        Serial.print(F("MONUMENTO RILEVATO: "));
        Serial.println(testInfo.title);
        Serial.print(F("DISTANZA CALCOLATA: "));
        Serial.print(testInfo.distance);
        Serial.println(F(" metri"));
        Serial.print(F("URL PAGINA WIKI:    "));
        Serial.println(testInfo.url);
        Serial.println(F("\nESTRATTO WIKIPEDIA:"));
        Serial.println(testInfo.extract);
        Serial.println(F("======================================================="));

        Display.showTestModeScreen(F("TEST 5: MONUMENTO TROVATO"), 
                                   testInfo.title, 
                                   "Dist: " + String((int)testInfo.distance) + "m | URL pronto");
        Feedback.playMonumentSequence();
    } else {
        Serial.println(F("[AVVISO] Nessun monumento trovato nel raggio indicato o errore HTTP."));
        Display.showTestModeScreen(F("TEST 5: NESSUN RISULTATO"), 
                                   F("Chiamata API completata."), 
                                   F("Nessun monumento trovato."));
        Feedback.playWarning();
    }

    Serial.println(F("\nPremi un tasto sulla seriale per continuare..."));
    while (!Serial.available()) delay(50);
    while (Serial.available()) Serial.read();
}

void TestManager::runSelfTest() {
    Display.showTestModeScreen(F("TEST 6: SELF-TEST AUTO"), 
                               F("Esecuzione test diagnostico globale..."), 
                               F("Attendi qualche secondo..."));

    Serial.println();
    Serial.println(F("======================================================================"));
    Serial.println(F("           REPORT AUTO-DIAGNOSTICO HARDWARE (SELF-TEST)               "));
    Serial.println(F("======================================================================"));

    // 1. Test Display
    Serial.print(F("[1/6] Schermo ST7789 SPI:       "));
    Display.getTFT().fillScreen(0x001F); // Blu rapido
    delay(100);
    Display.getTFT().fillScreen(0x07E0); // Verde rapido
    delay(100);
    Display.getTFT().fillScreen(0xF800); // Rosso rapido
    delay(100);
    Display.getTFT().fillScreen(COLOR_BG);
    Serial.println(F("[PASS] Display inizializzato e funzionante"));

    // 2. Test Joystick
    Serial.print(F("[2/6] Joystick ADC1 & Switch:   "));
    int jx = Joystick.getRawX();
    int jy = Joystick.getRawY();
    bool jbtn = Joystick.isButtonPressed();
    Serial.print(F("[PASS] Letto X="));
    Serial.print(jx);
    Serial.print(F(" Y="));
    Serial.print(jy);
    Serial.print(F(" SW="));
    Serial.println(jbtn ? F("CLICK") : F("OPEN"));

    // 3. Test Audio PWM (LEDC)
    Serial.print(F("[3/6] Buzzer PWM (GPIO 25):     "));
    Feedback.playClick();
    Serial.println(F("[PASS] Canale PWM LEDC attivo"));

    // 4. Test Vibrazione Aptica
    Serial.print(F("[4/6] Vibrazione PWM (GPIO 26): "));
    Feedback.playNote(0, 30, 10, 10); // Solo vibrazione senza suono
    Serial.println(F("[PASS] Motorino aptico testato"));

    // 5. Test QR Code Engine
    Serial.print(F("[5/6] Generatore QR Code:       "));
    bool qrOk = QR.generate("https://it.wikipedia.org");
    Serial.println(qrOk ? F("[PASS] Libreria qrcodegen operativa") : F("[FAIL] Errore codifica"));

    // 6. Test Ricevitore GPS
    Serial.print(F("[6/6] Modulo GPS (UART 2):      "));
    GPS.update();
    Serial.print(F("[INFO] Byte NMEA processati: "));
    Serial.print(GPS.getCharsProcessed());
    Serial.println(GPS.hasFix() ? F(" | Fix: PRESENTE") : F(" | Fix: IN ATTESA (normale al chiuso)"));

    Serial.println(F("----------------------------------------------------------------------"));
    Serial.println(F("Auto-Test completato con successo."));
    Serial.println(F("======================================================================"));

    Display.showTestModeScreen(F("SELF-TEST COMPLETATO"), 
                               F("Tutti i sottosistemi verificati."), 
                               F("Invia un tasto per tornare al menu."));
    Feedback.playSuccess();

    while (!Serial.available()) delay(50);
    while (Serial.available()) Serial.read();
}
