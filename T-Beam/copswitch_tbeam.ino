#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display parameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // I2C address for the SSD1306 display (0x3C or 0x3D)

// Create an instance of the display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* hassServer = "http://your_home_assistant_server";
const char* hassToken = "your_long_lived_access_token";

bool deviceFound = false;
unsigned long lastDetectedTimestamp = 0;
const unsigned long detectionInterval = 60000; // 1 minute in milliseconds
const char* uniqueEntityId = "switch.esp32_ble_axon_detector"; // Unique entity ID
const char* friendlyName = "Police Detector"; // Friendly name for the switch

void displayMessage(const char* message) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(message);
    display.display();
    delay(3000); // Display the message for 3 seconds
    display.clearDisplay();
    display.display();
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        String address = advertisedDevice.getAddress().toString().c_str();
        if (address.startsWith("00:25:df")) {
            deviceFound = true;
            lastDetectedTimestamp = millis();
            displayMessage("Police Detected");
        }
    }
};

void setup() {
    Serial.begin(115200);

    // Initialize OLED display
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    display.display();
    delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
        displayMessage("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    displayMessage("Connected to WiFi");

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
    String url = String(hassServer) + "/api/states/" + uniqueEntityId;
    String payload = "{\"state\": \"";
    payload += (state ? "on" : "off");
    payload += "\", \"attributes\": {\"friendly_name\": \"";
    payload += friendlyName;
    payload += "\"}}";

    http.begin(url);
    http.addHeader("Authorization", String("Bearer ") + hassToken); // Ensure Bearer token is correct
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        if (httpResponseCode == 401) {
            displayMessage("401 Unauthorized");
        }
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

    // Check Wi-Fi connection status
    if (WiFi.status() != WL_CONNECTED) {
        displayMessage("WiFi Disconnected");
        // Try to reconnect
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Reconnecting to WiFi...");
            displayMessage("Reconnecting to WiFi...");
        }
        Serial.println("Reconnected to WiFi");
        displayMessage("Reconnected to WiFi");
    }
}
