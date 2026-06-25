LED Timeline - skeleton

This repository branch contains a minimal skeleton to control WS2812 LED strips from an ESP32 using a WebSocket JSON protocol and a simple web UI to play MP3s and send cues.

What's included (branch: led-timeline-skeleton)
- firmware/main.ino - Arduino/PlatformIO sketch (LED data pin = 2)
- firmware/platformio.ini - PlatformIO envs for esp32dev and esp32c3-dev
- web/index.html - simple web UI (WaveSurfer + WebSocket) to load MP3 and send cues
- .gitignore

Quick start - PlatformIO
1. Clone the repo and checkout branch led-timeline-skeleton
   git clone https://github.com/gorrosled-oss/gorrosled-oss.git
   cd gorrosled-oss
   git checkout led-timeline-skeleton

2. Open the `firmware` folder in VSCode + PlatformIO or run:
   pio run -e esp32dev -t upload

3. Edit `firmware/main.ino` and set your WiFi SSID/PASSWORD or implement provisioning.
4. Connect your LED strip data pin to GPIO 2 and common GND. Use appropriate 5V power supply.

Quick start - Arduino IDE
1. Open `firmware/main.ino` in Arduino IDE.
2. Install libraries: FastLED, WebSockets, ArduinoJson
3. Select your ESP32 board and upload.

Using the web UI
1. Serve the `web` folder (or open `web/index.html` directly) and provide your ESP WebSocket URL when prompted (eg ws://192.168.4.1:81).
2. Load an MP3 file, press Play, pick an effect/color and press "Enviar Cue ahora" to send a cue to the ESP.

Next steps (suggested)
- Implement AP provisioning or WiFiManager for easier setup
- Add timeline editor with draggable regions and export/import JSON
- Add more effects and allow parameterized effect messages
- Add authentication or simple token exchange for security

Notes
- LED_PIN is set to GPIO 2 in the firmware. Change if needed.
- Power requirements: each WS2812 pixel can draw up to ~60mA at full white.

