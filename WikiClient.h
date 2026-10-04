#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "config.h"

struct MonumentInfo {
    bool valid = false;
    String title = "";
    float distance = 0.0f;
    float lat = 0.0f;
    float lon = 0.0f;
    String extract = "";
    String url = "";
};

class WikiClient {
public:
    WikiClient();

    void init(const char* ssid = DEFAULT_WIFI_SSID, const char* password = DEFAULT_WIFI_PASS);
    bool connect(uint32_t timeoutMs = 8000);
    void disconnect();
    bool isConnected() const;

    String getSSID() const;
    String getIP() const;
    int getRSSI() const;

    // Ricerca monumento più vicino con coordinate e raggio
    bool searchNearby(float lat, float lon, int radiusMeters, MonumentInfo &result);

    // Esegue una ricerca di test (usando coordinate fisse o attuali)
    bool testConnection(MonumentInfo &testResult);

    static String urlEncode(const String &str);

private:
    String _ssid;
    String _password;

    bool fetchExtract(const String &title, MonumentInfo &info);
};

extern WikiClient Wiki;
