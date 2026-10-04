#include "GPSManager.h"

GPSManager GPS;

GPSManager::GPSManager()
    : _gpsSerial(1), _lastLat(0.0f), _lastLon(0.0f),
      _firstFixAcquired(false), _simMode(false),
      _simLat(FALLBACK_LAT), _simLon(FALLBACK_LON) {}

void GPSManager::init() {
    _gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

void GPSManager::update() {
    while (_gpsSerial.available() > 0) {
        char c = _gpsSerial.read();
        _gps.encode(c);
    }
}

bool GPSManager::hasFix() {
    if (_simMode) return true;
    return (_gps.location.isValid() && _gps.location.age() < 3000);
}

float GPSManager::getLat() {
    if (_simMode) return _simLat;
    return (float)_gps.location.lat();
}

float GPSManager::getLon() {
    if (_simMode) return _simLon;
    return (float)_gps.location.lng();
}

int GPSManager::getSatellites() {
    if (_simMode) return 9;
    return _gps.satellites.value();
}

float GPSManager::getHDOP() {
    if (_simMode) return 1.0f;
    return _gps.hdop.hdop();
}

float GPSManager::getAltitude() {
    if (_simMode) return 42.0f;
    return (float)_gps.altitude.meters();
}

float GPSManager::getSpeedKmh() {
    if (_simMode) return 0.0f;
    return (float)_gps.speed.kmph();
}

float GPSManager::getCourse() {
    if (_simMode) return 0.0f;
    return (float)_gps.course.deg();
}

uint32_t GPSManager::getCharsProcessed() {
    return _gps.charsProcessed();
}

bool GPSManager::checkAndCommitMovement(float thresholdMeters) {
    if (!hasFix()) return false;

    float currentLat = getLat();
    float currentLon = getLon();

    if (!_firstFixAcquired) {
        _lastLat = currentLat;
        _lastLon = currentLon;
        _firstFixAcquired = true;
        return true; // Primo fix acquisito: forza prima ricerca
    }

    double dist = TinyGPSPlus::distanceBetween(currentLat, currentLon, _lastLat, _lastLon);
    if (dist >= thresholdMeters) {
        _lastLat = currentLat;
        _lastLon = currentLon;
        return true;
    }

    return false;
}

void GPSManager::setSimulationMode(bool enabled, float simLat, float simLon) {
    _simMode = enabled;
    _simLat = simLat;
    _simLon = simLon;
    if (enabled && !_firstFixAcquired) {
        _firstFixAcquired = true;
        _lastLat = simLat;
        _lastLon = simLon;
    }
}

bool GPSManager::isSimulationMode() const {
    return _simMode;
}

TinyGPSPlus& GPSManager::getRawGPS() {
    return _gps;
}

HardwareSerial& GPSManager::getSerial() {
    return _gpsSerial;
}
