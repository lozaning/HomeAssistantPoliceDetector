#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* hassServer = "http://YOUR_HASS_SERVER_IP:8123";
const char* hassToken = "Bearer YOUR_LONG_LIVED_ACCESS_TOKEN";

bool deviceFound = false;
unsigned long lastDetectedTimestamp = 0;
const unsigned long detectionInterval = 60000; // 1 minute in milliseconds

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        String address = advertisedDevice.getAddress().toString().c_str();
        if (address.startsWith("00:25:df", false)) {
            deviceFound = true;
            lastDetectedTimestamp = millis();
        }
    }
};

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Initialize BLE
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

void updateHomeAssistantSwitch(bool state) {
    HTTPClient http;
    String url = String(hassServer) + "/api/states/switch.esp32c3_ble_switch";
    String payload = "{\"state\": \"" + (state ? "on" : "off") + "\"}";

    http.begin(url);
    http.addHeader("Authorization", hassToken);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}

void loop() {
    deviceFound = false;

    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->start(5, false); // Scan for 5 seconds

    if (deviceFound || (millis() - lastDetectedTimestamp <= detectionInterval)) {
        updateHomeAssistantSwitch(true);
    } else {
        updateHomeAssistantSwitch(false);
    }

    delay(5000); // Delay for 5 seconds before next scan
}
