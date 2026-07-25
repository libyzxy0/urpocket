#ifndef APPNETWORKMANAGER_H
#define APPNETWORKMANAGER_H

class AppNetworkManager {
private:
  const char* ssid = "OrangeCat";
  const char* password = "myorange32";
  const char* ntpServer = "pool.ntp.org";
  const long gmtOffset_sec = 8 * 3600;
  const int daylightOffset_sec = 0;
  bool wifiEnabled = false;

public:
  bool isWifiEnabled() const { return wifiEnabled; }
  void toggleWiFi() {
    wifiEnabled = !wifiEnabled;
    if (wifiEnabled) {
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    } else {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
    }
  }
  String fetchHttpsApi(const char* url) {
    if (WiFi.status() != WL_CONNECTED) {
      return "Error: No WiFi";
    }
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    if (http.begin(client, url)) {
      int httpCode = http.GET();
      String payload = "";
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          payload = http.getString();
        } else {
          payload = "HTTP Error: " + String(httpCode);
        }
      } else {
        payload = "Conn Failed: " + String(http.errorToString(httpCode));
      }
      http.end();
      return payload;
    }
    return "Error: Invalid URL";
  }
};

#endif // APPNETWORKMANAGER_H
