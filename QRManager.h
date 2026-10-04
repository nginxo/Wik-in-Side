#pragma once

#include <Arduino.h>
#include <Adafruit_ST7789.h>
#include "qrcodegen.h"
#include "config.h"

// Dimensione buffer sicura: Versione 7 supporta fino a ~150 caratteri (più che sufficiente per URL Wikipedia)
#define QR_MAX_VERSION  7
#define QR_BUFFER_SIZE  qrcodegen_BUFFER_LEN_FOR_VERSION(QR_MAX_VERSION)

class QRManager {
public:
    QRManager();

    // Genera il QR code nel buffer interno o fornito
    bool generate(const char *text);

    // Stampa su porta Seriale (fedele a qrGen.c con blocchi ASCII / UTF-8)
    void printToSerial(Print &out = Serial);

    // Renderizza graficamente sul display TFT ST7789
    void drawToDisplay(Adafruit_ST7789 &tft, int centerX, int centerY, int maxSize = 180, 
                       uint16_t fgColor = 0x0000, uint16_t bgColor = 0xFFFF);

    int getSize() const;
    bool isValid() const;
    const char* getLastText() const;

private:
    uint8_t _qrData[QR_BUFFER_SIZE];
    uint8_t _tempBuffer[QR_BUFFER_SIZE];
    bool _hasValidQR;
    int _qrSize;
    String _lastText;
};

extern QRManager QR;
