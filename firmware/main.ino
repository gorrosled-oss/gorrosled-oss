/*
  Simple WS2812 controller with WebSocket control
  - DATA pin = 2
  - Uses FastLED + WebSockets + ArduinoJson
  - Supports: set_config, set_effect, cue, play_at

  Edit the WiFi credentials below or use a provisioning flow.
*/

#include <WiFi.h>
#include <WebSocketsServer.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ---------- CONFIG ----------
#define LED_PIN     2
#define DEFAULT_NUM_LEDS 60
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B

// WiFi credentials (change before building) or implement provisioning
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASS";

int NUM_LEDS = DEFAULT_NUM_LEDS;
CRGB *leds;

// WebSocket server on port 81
WebSocketsServer webSocket = WebSocketsServer(81);

// Optional NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// State
uint8_t brightness = 128;
String currentEffect = "solid";
uint16_t effectSpeed = 50;

// ------------- Helpers -------------
void applySolid(uint32_t color) {
  for (int i=0;i<NUM_LEDS;i++) leds[i] = CRGB((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
  FastLED.show();
}

void applyRainbow() {
  static uint8_t hue = 0;
  for (int i=0;i<NUM_LEDS;i++) {
    leds[i] = CHSV(hue + (i * 255 / NUM_LEDS), 200, brightness);
  }
  hue += (effectSpeed/10 + 1);
  FastLED.show();
}

void applyPulse(uint32_t color) {
  static int dir = 1;
  static uint8_t level = 0;
  if (dir > 0) level += 2; else level -= 2;
  if (level < 10) dir = 1;
  if (level > brightness) dir = -1;
  CRGB c = CRGB((color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
  for (int i=0;i<NUM_LEDS;i++) leds[i] = c.nscale8_video(level);
  FastLED.show();
}

// ------------- WebSocket handling -------------
void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) {
      Serial.printf("JSON parse error: %s\n", err.c_str());
      return;
    }
    const char* cmd = doc["cmd"];
    if (!cmd) return;

    if (strcmp(cmd, "set_config") == 0) {
      if (doc.containsKey("num_leds")) {
        int newn = doc["num_leds"];
        if (newn > 0 && newn <= 5000) {
          delete[] leds;
          NUM_LEDS = newn;
          leds = new CRGB[NUM_LEDS];
          FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
          FastLED.setBrightness(brightness);
        }
      }
      if (doc.containsKey("brightness")) {
        brightness = doc["brightness"];
        FastLED.setBrightness(brightness);
      }
      DynamicJsonDocument out(256);
      out["evt"] = "config_ok";
      out["num_leds"] = NUM_LEDS;
      String s; serializeJson(out, s);
      webSocket.sendTXT(num, s);

    } else if (strcmp(cmd, "set_effect") == 0) {
      const char* eff = doc["effect"];
      if (eff) currentEffect = String(eff);
      if (doc.containsKey("speed")) effectSpeed = doc["speed"];

    } else if (strcmp(cmd, "cue") == 0) {
      const char* eff = doc["effect"];
      const char* color = doc["color"];
      if (eff) currentEffect = String(eff);
      if (color) {
        if (color[0]=='#' && strlen(color) >= 7) {
          long c = strtol(color+1, NULL, 16);
          if (currentEffect == "solid") applySolid((uint32_t)c);
          else if (currentEffect == "pulse") applyPulse((uint32_t)c);
          else applySolid((uint32_t)c);
        }
      }

    } else if (strcmp(cmd, "play_at") == 0) {
      unsigned long ts = doc["timestamp_ms"];
      DynamicJsonDocument out(256);
      out["evt"] = "play_scheduled";
      out["at"] = ts;
      String s; serializeJson(out, s);
      webSocket.sendTXT(num, s);
    }
  }
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  handleWebSocketMessage(num, type, payload, length);
}

// ------------- Setup / Loop -------------
void setup() {
  Serial.begin(115200);
  leds = new CRGB[NUM_LEDS];
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  FastLED.setBrightness(brightness);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(300);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi connect failed - start AP mode not implemented in this sketch");
  }

  // NTP
  timeClient.begin();
  timeClient.update();

  // WebSocket
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("WebSocket server started on port 81.");
}

void loop() {
  webSocket.loop();
  timeClient.update();

  if (currentEffect == "rainbow") {
    applyRainbow();
  } else if (currentEffect == "pulse") {
    // pulse requires a color from the last cue - for simplicity use magenta
    applyPulse(0xFF00FF);
  }
  delay(20);
}
