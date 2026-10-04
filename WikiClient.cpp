#include "WikiClient.h"

WikiClient Wiki;

WikiClient::WikiClient()
    : _ssid(DEFAULT_WIFI_SSID), _password(DEFAULT_WIFI_PASS) {}

void WikiClient::init(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
}

bool WikiClient::connect(uint32_t timeoutMs) {
    if (WiFi.status() == WL_CONNECTED) return true;

    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid.c_str(), _password.c_str());

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start < timeoutMs)) {
        delay(200);
    }

    return (WiFi.status() == WL_CONNECTED);
}

void WikiClient::disconnect() {
    WiFi.disconnect();
}

bool WikiClient::isConnected() const {
    return (WiFi.status() == WL_CONNECTED);
}

String WikiClient::getSSID() const {
    return _ssid;
}

String WikiClient::getIP() const {
    return WiFi.localIP().toString();
}

int WikiClient::getRSSI() const {
    return WiFi.RSSI();
}

String WikiClient::urlEncode(const String &str) {
    String encoded = "";
    char c;
    char code0;
    char code1;
    for (unsigned int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (isalnum(c)) {
            encoded += c;
        } else if (c == ' ') {
            encoded += "%20";
        } else if (c == '_') {
            encoded += "_";
        } else {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            encoded += '%';
            encoded += code0;
            encoded += code1;
        }
    }
    return encoded;
}

bool WikiClient::searchNearby(float lat, float lon, int radiusMeters, MonumentInfo &result) {
    result.valid = false;
    if (!isConnected()) {
        if (!connect(4000)) return false;
    }

    WiFiClientSecure secureClient;
    secureClient.setInsecure(); // Consente connessione HTTPS senza richiedere certificati radice salvati in flash

    HTTPClient http;
    String url = "https://";
    url += WIKI_LANG;
    url += ".wikipedia.org/w/api.php?action=query&list=geosearch&gscoord=";
    url += String(lat, 6) + "|" + String(lon, 6);
    url += "&gsradius=" + String(radiusMeters);
    url += "&gslimit=1&format=json";

    if (!http.begin(secureClient, url)) {
        return false;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        return false;
    }

    JsonArray results = doc["query"]["geosearch"];
    if (results.isNull() || results.size() == 0) {
        return false;
    }

    result.title = results[0]["title"].as<String>();
    result.distance = results[0]["dist"].as<float>();
    result.lat = results[0]["lat"].as<float>();
    result.lon = results[0]["lon"].as<float>();
    result.url = String("https://") + WIKI_LANG + ".wikipedia.org/wiki/" + urlEncode(result.title);
    result.extract = "";

    // Recupera la descrizione / extract dell'articolo
    fetchExtract(result.title, result);

    result.valid = true;
    return true;
}

bool WikiClient::fetchExtract(const String &title, MonumentInfo &info) {
    WiFiClientSecure secureClient;
    secureClient.setInsecure();

    HTTPClient http;
    String encodedTitle = urlEncode(title);
    String url = "https://";
    url += WIKI_LANG;
    url += ".wikipedia.org/w/api.php?action=query&prop=extracts&exintro=1&explaintext=1&format=json&titles=";
    url += encodedTitle;

    if (!http.begin(secureClient, url)) return false;

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) return false;

    JsonObject pages = doc["query"]["pages"].as<JsonObject>();
    for (JsonPair kv : pages) {
        if (kv.value().containsKey("extract")) {
            info.extract = kv.value()["extract"].as<String>();
            return true;
        }
    }

    return false;
}

bool WikiClient::testConnection(MonumentInfo &testResult) {
    return searchNearby(FALLBACK_LAT, FALLBACK_LON, 500, testResult);
}
