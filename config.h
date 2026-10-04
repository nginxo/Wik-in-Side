#pragma once

#include <Arduino.h>

/*******************************************************************************
 *                          GeoOS / WikiOS per ESP32
 *               Configurazione di Sistema, Pinout e Parametri
 ******************************************************************************/

// Informazioni Firmware
#define OS_NAME             "WikinSide OS"
#define OS_VERSION          "1.0.0"
#define OS_AUTHOR           "exertia Group"

// =============================================================================
// PINOUT HARDWARE (Ottimizzato per ESP32, esente da conflitti e compatibile WiFi)
// =============================================================================

/*
 * NOTE IMPORTANTI SUL PINOUT:
 * 1. Lo schermo ST7789 usa il bus SPI hardware (VSPI).
 * 2. Il GPS usa la UART hardware 2 (Serial2 sui pin 16 e 17).
 * 3. Il Joystick USA ESCLUSIVAMENTE I PIN DELL'ADC1 (GPIO 34, 35, 32).
 *    I pin ADC2 (es. GPIO 0, 2) NON funzionano quando il modulo Wi-Fi è attivo!
 * 4. Buzzer e Vibrazione usano canali PWM (LEDC) su pin GPIO dedicati (25 e 26).
 */

// --- Display ST7789 (Interfaccia SPI) ---
#define TFT_CS              16    // Chip Select display
#define TFT_DC              27   // Data / Command display
#define TFT_RST             17    // Reset display
#define TFT_MOSI            19   // SPI MOSI (SDA)
#define TFT_SCLK            23   // SPI Clock (SCL)
#define TFT_BL              -1   // Retroilluminazione (-1 se collegato direttamente a 3.3V)
#define TFT_ROTATION        2  // 1 = Landscape (320x240), 3 = Landscape invertito

#define SCREEN_WIDTH        320
#define SCREEN_HEIGHT       240

// --- Modulo GPS (UART Serial2) ---
#define GPS_RX_PIN          4   // Connettere al TX del GPS (es. NEO-6M)
#define GPS_TX_PIN          5   // Connettere al RX del GPS
#define GPS_BAUD_RATE       9600 // Baud rate standard NMEA

// --- Joystick Analogico con Pulsante ---
// NOTA: Usiamo ADC1 (GPIO 32..39) per totale compatibilità con il Wi-Fi
#define JOY_VRX_PIN         34   // Asse X (ADC1_CH6 - solo input)
#define JOY_VRY_PIN         35   // Asse Y (ADC1_CH7 - solo input)
#define JOY_SW_PIN          32   // Pulsante integrato (con INPUT_PULLUP interno)
#define JOY_DEADZONE        350  // Zona morta per evitare drift dello stick (0..2048)
#define JOY_INVERT_X        false
#define JOY_INVERT_Y        false

// --- Audio (Buzzer) & Feedback Aptico (Motorino Vibrazione) ---
#define BUZZER_PIN          15   // Pin Buzzer (PWM LEDC)
#define VIBRO_PIN           14   // Pin Motorino vibrazione (PWM LEDC)

#define BUZZER_CHANNEL_FREQ 2000 // Frequenza base buzzer (Hz)
#define VIBRO_CHANNEL_FREQ  1000 // Frequenza base vibrazione (Hz)
#define PWM_RESOLUTION      8    // Risoluzione 8 bit (0-255)

// Parametri curva inviluppo (derivati da audiovibration.c)
#define NOTE_DURATION_MS    50
#define ATTACK_MS           40
#define RELEASE_MS          60
#define MIN_VIBRO_PWM       150  // Soglia minima per vincere l'inerzia del motorino
#define MAX_VIBRO_PWM       255  // Picco massimo vibrazione

// Sequenza melodica notifica monumento (da audiovibration.c)
const int MONUMENT_NOTES[] = { 888, 1142, 1333 };
const int NUM_MONUMENT_NOTES = sizeof(MONUMENT_NOTES) / sizeof(MONUMENT_NOTES[0]);

// =============================================================================
// CONFIGURAZIONE RETE WI-FI & WIKIPEDIA API
// =============================================================================
#define DEFAULT_WIFI_SSID       "Fablab4"
#define DEFAULT_WIFI_PASS       "fablabme"

#define WIKI_LANG               "it"
#define WIKI_DEFAULT_RADIUS     100   // Raggio ricerca iniziale in metri
#define WIKI_MIN_RADIUS         50
#define WIKI_MAX_RADIUS         2000
#define WIKI_RADIUS_STEP        50

#define GPS_MOVE_THRESHOLD_M    50.0f // Spostamento minimo in metri per nuovo query automatico

// Coordinate fallback e demo per test indoor (Monte Fuji, Honshu, Giappone)
#define FUJI_LAT                35.360556f
#define FUJI_LON                138.727778f
#define FALLBACK_LAT            FUJI_LAT
#define FALLBACK_LON            FUJI_LON

// Timeout pressione prolungata pulsante per attivare Fallback Test Demo Monte Fuji (3 secondi)
#define FUJI_DEMO_HOLD_MS       3000

// Timeout spegnimento schermo in assenza di monumenti vicini (10 secondi)
#define SCREEN_TIMEOUT_MS       10000


// =============================================================================
// SERIAL TEST MODE
// =============================================================================
#define SERIAL_BAUD_RATE        115200
#define TEST_COMMAND_TRIGGER    "TEST"

// =============================================================================
// TAVOLOZZA COLORI DISPLAY (RGB565 per ST7789)
// =============================================================================
#define COLOR_BG            0x0821  // Blu scuro quasi nero (#080c10)
#define COLOR_HEADER        0x1927  // Blu ardesia scuro (#192538)
#define COLOR_TEXT_WHITE    0xFFFF  // Bianco puro
#define COLOR_TEXT_MUTED    0x9CD3  // Grigio chiaro / azzurrino
#define COLOR_ACCENT_CYAN   0x06FD  // Ciano brillante (#00dfff)
#define COLOR_ACCENT_AMBER  0xFD60  // Ambra / Giallo caldo (#ffaa00)
#define COLOR_ACCENT_GREEN  0x2E65  // Verde smeraldo (#22cc55)
#define COLOR_ACCENT_RED    0xF986  // Rosso allerta (#f43f5e)
#define COLOR_CARD_BG       0x18C4  // Sfondo schede / card (#181824)
#define COLOR_CARD_BORDER   0x31E9  // Bordo schede
