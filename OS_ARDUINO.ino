#include <Arduino.h>
#include "config.h"
#include "FeedbackManager.h"
#include "JoystickManager.h"
#include "GPSManager.h"
#include "WikiClient.h"
#include "QRManager.h"
#include "DisplayManager.h"
#include "TestManager.h"



MonumentInfo currentMonument;
uint32_t lastSearchCheck = 0;
uint32_t lastWifiRetry = 0;
bool initialSearchDone = false;

// display e demo
bool isDemoActive = false;
uint32_t screenWakeTimeout = 0;
bool isScreenAwake = true;

void loadMonteFujiDemo() {
    currentMonument.valid = true;
    currentMonument.title = F("Monte Fuji");
    currentMonument.distance = 350.0f; // 350 m stimati (visibile sul radar max 500m)
    currentMonument.lat = FUJI_LAT;
    currentMonument.lon = FUJI_LON;
    currentMonument.extract = F(
        "Il Fuji (Fuji-san) e' un vulcano alto 3776 m situato sull'isola giapponese di Honshu. "
        "In Europa e' noto anche con il nome di Fujiyama. E' il monte piu' alto del Giappone ed e' "
        "considerato una delle tre montagne sacre (Sanreizan), insieme al monte Tate e al monte Haku, "
        "meta di pellegrinaggio shintoista almeno una volta nella vita. Con la sua cima innevata "
        "per dieci mesi all'anno, e' uno dei simboli del Giappone e fa parte dell'elenco nazionale "
        "dei luoghi speciali per valore storico e bellezza paesaggistica. Dal 2013 e' patrimonio dell'umanita' "
        "UNESCO, che riconosce nella sua area venticinque siti di interesse culturale."
    );
    currentMonument.url = F("https://it.wikipedia.org/wiki/Monte_Fuji");

    isDemoActive = true;

    // Generazione del codice QR
    QR.generate(currentMonument.url.c_str());

    // Imposta coordinate
    GPS.setSimulationMode(true, 35.358000f, 138.725000f);

    // Risveglio schermo
    Display.wakeup();
    isScreenAwake = true;
    Feedback.playMonumentSequence();

    // Ritorno alla vista Esplora
    Display.resetScroll();
    Display.switchView(VIEW_EXPLORE);
}

void unloadDemo() {
    isDemoActive = false;
    currentMonument.valid = false;
    currentMonument.title = "";
    currentMonument.extract = "";
    currentMonument.url = "";
    currentMonument.distance = 0.0f;
    currentMonument.lat = 0.0f;
    currentMonument.lon = 0.0f;

    GPS.setSimulationMode(false);
    Feedback.playWarning();
    Display.switchView(VIEW_EXPLORE);

    // Poiche' non c'e' piu' un monumento vicino, lo schermo si spegnera' dopo il timeout
    screenWakeTimeout = millis() + SCREEN_TIMEOUT_MS;

}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(100);

    // avvio dell'OS
    Feedback.init();
    Display.init();
    Display.showBootScreen(F("Inizializzazione periferiche..."), 15);
    Feedback.playBootJingle();
    Display.showBootScreen(F("Calibrazione Joystick..."), 35);
    Joystick.init();
    Display.showBootScreen(F("Avvio modulo GPS..."), 55);
    GPS.init();
    Display.showBootScreen(F("Connessione alla rete Wi-Fi..."), 75);
    Wiki.init(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);
    bool wifiOk = Wiki.connect(2500); // Tentativo iniziale veloce
    Diagnostics.init();

    Display.showBootScreen(F("Dispositivo pronto"), 100);
    delay(100);

    Display.getTFT().fillScreen(COLOR_BG); 
    Display.switchView(VIEW_EXPLORE);

    // spegnimento schermo per inutilizzo
    screenWakeTimeout = millis() + SCREEN_TIMEOUT_MS;
    isScreenAwake = true;
}

void loop() {
    Diagnostics.checkSerialTrigger();

    if (Diagnostics.isInTestMode()) {
        Diagnostics.update();
        return; // Tutto il resto viene sospeso mentre siamo in test mode
    }

    // aggiorna sensori
    GPS.update();
    Joystick.update();


    bool joyMoved = (Joystick.getDirection() != JOY_DIR_NONE) || Joystick.anyMovement();
    bool btnDown = Joystick.isButtonPressed();
    bool userActive = joyMoved || btnDown;

    if (userActive) {
        if (!isScreenAwake) {
            Display.wakeup();
            isScreenAwake = true;
            Serial.println(F("[Power] Joystick/Pulsante mosso: risveglio schermo per 10s."));
        }
        // Rinnova il timer di accensione dello schermo
        screenWakeTimeout = millis() + SCREEN_TIMEOUT_MS;
    }


    if (Joystick.buttonHeldTrigger(FUJI_DEMO_HOLD_MS)) {
        if (!isDemoActive) {
            loadMonteFujiDemo();
        } else {
            unloadDemo();
        }
    }


    // 5. NAVIGAZIONE INTERFACCIA CON JOYSTICK 

    if (isScreenAwake) {
        // Cambio vista (Sinistra / Destra)
        if (Joystick.justMoved(JOY_DIR_LEFT)) {
            Display.previousView();
            Feedback.playClick();
        } else if (Joystick.justMoved(JOY_DIR_RIGHT)) {
            Display.nextView();
            Feedback.playClick();
        }

        // Scorrimento testi (Su / Giu)
        if (Display.getCurrentView() == VIEW_ARTICLE) {
            if (Joystick.isHoldRepeating(JOY_DIR_UP)) {
                Display.scrollUp();
            } else if (Joystick.isHoldRepeating(JOY_DIR_DOWN)) {
                Display.scrollDown();
            }
        }

        // Click breve pulsante stick (rilasciato prima di 2.5s)
        if (Joystick.buttonClicked()) {
            Feedback.playClick();
            OSView current = Display.getCurrentView();

            switch (current) {
                case VIEW_EXPLORE:
                    if (currentMonument.valid) {
                        Display.switchView(VIEW_ARTICLE);
                    } else {
                        // Se non c'è ancora un monumento, forza una ricerca manuale
                        if (Wiki.isConnected()) {
                            Serial.println(F("[OS] Ricerca forzata da pulsante stick..."));
                            float searchLat = GPS.hasFix() ? GPS.getLat() : FALLBACK_LAT;
                            float searchLon = GPS.hasFix() ? GPS.getLon() : FALLBACK_LON;
                            if (Wiki.searchNearby(searchLat, searchLon, WIKI_DEFAULT_RADIUS, currentMonument)) {
                                Feedback.playMonumentSequence();
                                QR.generate(currentMonument.url.c_str());
                                Display.switchView(VIEW_EXPLORE);
                            } else {
                                Feedback.playWarning();
                            }
                        } else {
                            Feedback.playWarning();
                        }
                    }
                    break;

                case VIEW_ARTICLE:
                    Display.switchView(VIEW_QR);
                    break;

                case VIEW_QR:
                    Display.switchView(VIEW_RADAR);
                    break;

                case VIEW_RADAR:
                    Display.switchView(VIEW_EXPLORE);
                    break;

                case VIEW_SETTINGS:
                    GPS.setSimulationMode(!GPS.isSimulationMode());
                    Feedback.playClick();
                    Display.switchView(VIEW_SETTINGS); // Forza ridisegno
                    break;

                default:
                    break;
            }
        }
    }


    // LOGICA GPS

    bool needSearch = false;

    if (!initialSearchDone) {
        if (GPS.hasFix()) {
            needSearch = true;
            initialSearchDone = true;
        }
    } else {
        if (GPS.checkAndCommitMovement(GPS_MOVE_THRESHOLD_M)) {
            needSearch = true;
            Serial.println(F("\n[GPS] Spostamento rilevato (>50m). Nuova ricerca Wikipedia..."));
        }
    }

    if (needSearch && Wiki.isConnected() && !isDemoActive) {
        float queryLat = GPS.getLat();
        float queryLon = GPS.getLon();

        Serial.print(F("[Wiki] Interrogazione per coordinate: "));
        Serial.print(queryLat, 5);
        Serial.print(F(", "));
        Serial.println(queryLon, 5);

        MonumentInfo newInfo;
        if (Wiki.searchNearby(queryLat, queryLon, WIKI_DEFAULT_RADIUS, newInfo)) {
            currentMonument = newInfo;
            QR.generate(currentMonument.url.c_str());
            Feedback.playMonumentSequence();
            Display.resetScroll();

            // Risveglia e mantiene acceso lo schermo
            Display.wakeup();
            isScreenAwake = true;
        }
    }

    // Riconnessione WiFi periodica in background se persa
    if (!Wiki.isConnected() && (millis() - lastWifiRetry > 15000)) {
        lastWifiRetry = millis();
        Wiki.connect(1500);
    }

    // STANDBY
    if (!currentMonument.valid) {
        // Nessun monumento vicino: dopo 10s di inattività lo schermo si spegne
        if (isScreenAwake && (millis() >= screenWakeTimeout)) {
            Display.sleep();
            isScreenAwake = false;
            Serial.println(F("[Power] Nessun monumento vicino: timeout 10s scaduto, schermo spento."));
        }
    } else {
        // C'è un monumento vicino: lo schermo deve rimanere acceso
        if (!isScreenAwake) {
            Display.wakeup();
            isScreenAwake = true;
        }
    }


    Display.update(
        currentMonument,
        Wiki.isConnected(),
        GPS.hasFix(),
        GPS.getSatellites(),
        GPS.getLat(),
        GPS.getLon()
    );

    delay(20); 
}
