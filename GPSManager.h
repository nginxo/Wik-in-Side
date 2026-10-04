#pragma once

#include <Arduino.h>
#include <TinyGPS++.h>
#include "config.h"

class GPSManager {
public:
    GPSManager();

    void init();
    void update();

    bool hasFix();
    float getLat();
    float getLon();
    int getSatellites();
    float getHDOP();
    float getAltitude();
    float getSpeedKmh();
    float getCourse();
    uint32_t getCharsProcessed();

    // Rilevamento spostamento significativo
    bool checkAndCommitMovement(float thresholdMeters = GPS_MOVE_THRESHOLD_M);

    // Modalità simulazione coordinate per test indoor
    void setSimulationMode(bool enabled, float simLat = FALLBACK_LAT, float simLon = FALLBACK_LON);
    bool isSimulationMode() const;

    // Accesso a TinyGPS++ e porta seriale per diagnostica NMEA
    TinyGPSPlus& getRawGPS();
    HardwareSerial& getSerial();

private:
    HardwareSerial _gpsSerial;
    TinyGPSPlus _gps;

    float _lastLat;
    float _lastLon;
    bool _firstFixAcquired;
    bool _simMode;
    float _simLat;
    float _simLon;
};

extern GPSManager GPS;
