# Home Assistant Police Detector for LilyGo T-Beam

An optimised version of Lozaning's police detector sketch for the LilyGo T-Beam with SSD1306 display support.


## Notes
- Be sure to erase your T-Beam using [Espressif's ESP Tool](https://espressif.github.io/esptool-js/). **'Erase All Flash Before The Sketch Upload' in the Arduino IDE doesn't work for some reason**.

- Remove the placeholders and add your SSID, network nassword, HA server URL and long lived access token. You can generate a new LLAT at the bottom of your profile page in Home Assistant.

- No brainer but it's best to use the [**legacy IDE**](https://www.arduino.cc/en/software). You can have weird issues whilst working with ESP32 boards on the newer versions.

- You can't change the entity ID or friendly name within Home Assistant. It's best to change it in the sketch if you desire.

- Set your partition scheme to 'Huge APP (3MB No OTA/1 MB SPIFFS)'. **If you don't do this you'll get an error saying that the sketch is too big**.

- The version of the ESP32 boards manager (3.0.1) does not have partition scheme options for the T-Beam at this time. [You'll have to manually add them to your boards.txt](https://github.com/LilyGO/TTGO-T-Beam/issues/29).