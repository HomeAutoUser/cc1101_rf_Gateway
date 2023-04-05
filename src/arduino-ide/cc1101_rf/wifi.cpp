#include "config.h"

void start_WLAN_AP(String ssid_ap, String password_ap);
void start_WLAN_STATION(String qssid, String qpass);
bool start_WLAN_WPS();
String WLAN_MAC_String(uint8_t* MAC);
String WLAN_encryptionType(uint8_t i);

#ifdef ARDUINO_ARCH_ESP32
void ESP32_WiFiEvent();
void ESP32_wpsInitConfig();
String ESP32_wpspin2string(uint8_t a[]);
#endif
