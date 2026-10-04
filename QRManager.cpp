#include "QRManager.h"

QRManager QR;

QRManager::QRManager()
    : _hasValidQR(false), _qrSize(0), _lastText("") {
    memset(_qrData, 0, sizeof(_qrData));
    memset(_tempBuffer, 0, sizeof(_tempBuffer));
}

bool QRManager::generate(const char *text) {
    if (!text || strlen(text) == 0) {
        _hasValidQR = false;
        return false;
    }

    _lastText = text;
    bool success = qrcodegen_encodeText(
        text,
        _tempBuffer,
        _qrData,
        qrcodegen_Ecc_LOW,
        qrcodegen_VERSION_MIN,
        QR_MAX_VERSION,
        qrcodegen_Mask_AUTO,
        true
    );

    if (success) {
        _hasValidQR = true;
        _qrSize = qrcodegen_getSize(_qrData);
    } else {
        _hasValidQR = false;
        _qrSize = 0;
    }

    return success;
}

void QRManager::printToSerial(Print &out) {
    if (!_hasValidQR) {
        out.println(F("[QR] Nessun QR code valido generato."));
        return;
    }

    int border = 2;
    out.println();
    out.println(F("========================================"));
    out.print(F("QR CODE GENERATO PER: "));
    out.println(_lastText);
    out.println(F("========================================"));

    for (int y = -border; y < _qrSize + border; y++) {
        for (int x = -border; x < _qrSize + border; x++) {
            bool color = qrcodegen_getModule(_qrData, x, y);
            // Stampa caratteri doppi ASCII compatibili con tutti i monitor seriali
            out.print(color ? "##" : "  ");
        }
        out.println();
    }
    out.println();
}

void QRManager::drawToDisplay(Adafruit_ST7789 &tft, int centerX, int centerY, int maxSize, 
                              uint16_t fgColor, uint16_t bgColor) {
    if (!_hasValidQR) return;

    int border = 2;
    int totalModules = _qrSize + 2 * border;

    int scale = maxSize / totalModules;
    if (scale < 1) scale = 1;

    int totalPixelSize = totalModules * scale;
    int startX = centerX - (totalPixelSize / 2);
    int startY = centerY - (totalPixelSize / 2);

    // Disegna la "Quiet Zone" (sfondo bianco) attorno al QR
    tft.fillRect(startX, startY, totalPixelSize, totalPixelSize, bgColor);

    // Disegna i moduli neri
    for (int y = 0; y < _qrSize; y++) {
        for (int x = 0; x < _qrSize; x++) {
            if (qrcodegen_getModule(_qrData, x, y)) {
                tft.fillRect(startX + (x + border) * scale,
                             startY + (y + border) * scale,
                             scale, scale, fgColor);
            }
        }
    }
}

int QRManager::getSize() const {
    return _qrSize;
}

bool QRManager::isValid() const {
    return _hasValidQR;
}

const char* QRManager::getLastText() const {
    return _lastText.c_str();
}
