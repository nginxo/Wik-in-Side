#pragma once

#include <Arduino.h>

/*******************************************************************************
 *                          wik(in)Side
 ******************************************************************************/

// Informazioni Firmware
#define OS_NAME             "WikinSide OS"
#define OS_VERSION          "1.0.0"
#define OS_AUTHOR           "exertia Group"

// =============================================================================
// PINOUT HARDWARE (Ottimizzato per ESP32, esente da conflitti e compatibile WiFi)
// =============================================================================



// --- Display
#define TFT_CS              16   
#define TFT_DC              27  
#define TFT_RST             17  
#define TFT_MOSI            19   // SPI MOSI (SDA)
#define TFT_SCLK            23   // SPI Clock (SCL)
#define TFT_BL              -1  
#define TFT_ROTATION        2  // 1 = Landscape (320x240)

#define SCREEN_WIDTH        320
#define SCREEN_HEIGHT       240

// --- Modulo GPS
#define GPS_RX_PIN          4   
#define GPS_TX_PIN          5  
#define GPS_BAUD_RATE       9600 

// --- Joystick
#define JOY_VRX_PIN         34  
#define JOY_VRY_PIN         35  
#define JOY_SW_PIN          32   
#define JOY_DEADZONE        350 
#define JOY_INVERT_X        false
#define JOY_INVERT_Y        false

#define BUZZER_PIN          15   
#define VIBRO_PIN           14   

#define BUZZER_CHANNEL_FREQ 2000 
#define VIBRO_CHANNEL_FREQ  1000 
#define PWM_RESOLUTION      8    

// AUDIO
#define NOTE_DURATION_MS    30
#define ATTACK_MS           40
#define RELEASE_MS          60
#define MIN_VIBRO_PWM       150  
#define MAX_VIBRO_PWM       255  

const int MONUMENT_NOTES[] = { 888, 1142, 1333 };
const int NUM_MONUMENT_NOTES = sizeof(MONUMENT_NOTES) / sizeof(MONUMENT_NOTES[0]);


// CONFIGURAZIONE RETE WI-FI

#define DEFAULT_WIFI_SSID       "Fablab4"
#define DEFAULT_WIFI_PASS       "fablabme"

#define WIKI_LANG               "it"
#define WIKI_DEFAULT_RADIUS     100   // Raggio ricerca iniziale in metri
#define WIKI_MIN_RADIUS         50
#define WIKI_MAX_RADIUS         2000
#define WIKI_RADIUS_STEP        50

#define GPS_MOVE_THRESHOLD_M    50.0f // Spostamento minimo in metri per nuovo query automatico

// demo Monte Fuji
#define FUJI_LAT                35.360556f
#define FUJI_LON                138.727778f
#define FALLBACK_LAT            FUJI_LAT
#define FALLBACK_LON            FUJI_LON
#define FUJI_DEMO_HOLD_MS       3000

// timeout spegnimento schermo
#define SCREEN_TIMEOUT_MS       10000


//test
#define SERIAL_BAUD_RATE        115200
#define TEST_COMMAND_TRIGGER    "TEST"


#define COLOR_BG            0x0821  
#define COLOR_HEADER        0x1927  
#define COLOR_TEXT_WHITE    0xFFFF  
#define COLOR_TEXT_MUTED    0x9CD3  
#define COLOR_ACCENT_CYAN   0x06FD  
#define COLOR_ACCENT_AMBER  0xFD60  
#define COLOR_ACCENT_GREEN  0x2E65  
#define COLOR_ACCENT_RED    0xF986  
#define COLOR_CARD_BG       0x18C4  
#define COLOR_CARD_BORDER   0x31E9  
