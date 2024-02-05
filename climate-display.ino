#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <InputDebounce.h>
#include <NewEncoder.h>
#include <HCSR04.h>
#include <arduino-timer.h>

#include <FastLED.h>

const char* host = "esp32";
const char* ssid = "AhoyThereCastaways";
const char* password = "97755056664745005086";

const char* mqtt_server = "192.168.178.43";

#include "web.h"
#include "ClimateDisplay.h"

#define LED_PIN 2
#define NUM_LEDS 6
#define ROTARY_PIN_A 16
#define ROTARY_PIN_B 17
#define BUTTON1_PIN 18
#define US_SENSOR_PIN_ECHO 15
#define US_SENSOR_PIN_TRIG 13

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define BUTTON_DEBOUNCE_DELAY 50

auto timer = timer_create_default();

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WebServer server(80);
CRGB leds[NUM_LEDS];
ClimateDisplay climateDisplay(leds, &display);

NewEncoder encoder(ROTARY_PIN_A, ROTARY_PIN_B, -20, 20, 0, 1);
int16_t prevEncoderValue;

static InputDebounce button1;

HCSR04 hc(US_SENSOR_PIN_TRIG, US_SENSOR_PIN_ECHO);
float hcCurrentDist;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // TODO: Do a better job of parsing this
    if(std::string(topic) == "climatedisplay/kitchen/humidity") {
        float humidity = atof((char*)payload);
        climateDisplay.setRoomHumidity(Room::Kitchen, atof((char*)payload));
    } else if(std::string(topic) == "climatedisplay/kitchen/temperature") {
        float temperature = atof((char*)payload);
        climateDisplay.setRoomTemperature(Room::Kitchen, atof((char*)payload));
    } else if(std::string(topic) == "climatedisplay/bathroom/humidity") {
        float humidity = atof((char*)payload);
        climateDisplay.setRoomHumidity(Room::Bathroom, atof((char*)payload));
    } else if(std::string(topic) == "climatedisplay/bathroom/temperature") {
        float temperature = atof((char*)payload);
        climateDisplay.setRoomTemperature(Room::Bathroom, atof((char*)payload));
    } else if(std::string(topic) == "climatedisplay/bedroom/humidity") {
        float humidity = atof((char*)payload);
        climateDisplay.setRoomHumidity(Room::Bedroom, atof((char*)payload));
    } else if(std::string(topic) == "climatedisplay/bedroom/temperature") {
        float temperature = atof((char*)payload);
        climateDisplay.setRoomTemperature(Room::Bedroom, atof((char*)payload));
    } else if(std::string(topic) == "climatedisplay/corridor/humidity") {
        float humidity = atof((char*)payload);
        climateDisplay.setRoomHumidity(Room::Corridor, atof((char*)payload));
    } else if(std::string(topic) == "climatedisplay/corridor/temperature") {
        float temperature = atof((char*)payload);
        climateDisplay.setRoomTemperature(Room::Corridor, atof((char*)payload));
    }
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("ESP32Client", "mosquitto", "mosquitto")) {
            Serial.println("connected");

            client.subscribe("climatedisplay/kitchen/humidity");
            client.subscribe("climatedisplay/kitchen/temperature");
            client.subscribe("climatedisplay/bedroom/humidity");
            client.subscribe("climatedisplay/bedroom/temperature");
            client.subscribe("climatedisplay/bathroom/humidity");
            client.subscribe("climatedisplay/bathroom/temperature");
            client.subscribe("climatedisplay/corridor/humidity");
            client.subscribe("climatedisplay/corridor/temperature");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void knobCallback(long value) {
    Serial.printf("Value: %i\n", value);
}

/*
 * setup function
 */
void setup(void) {
   pinMode(2, OUTPUT);
   FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  Serial.begin(115200);

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // MQTT Setup
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.begin();

  // MQTT and Web Timers
  timer.every(1, [](void *arg) -> bool {
    server.handleClient();
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    return true;
  });


  // Setup OLED
  Wire.begin(22,19);
  if(!display.begin(0x3C, true)) {
    Serial.println(F("SH1106: allocation failed"));
    for(;;);
  }

  // Setup Button
  pinMode(BUTTON1_PIN, INPUT);
  button1.registerCallbacks(nullptr, [](uint8_t pin) {
    Serial.println("Button 1 released");
    climateDisplay.fadeDisplayLED(LED::Humidity, CRGB::White);
  });
  button1.setup(BUTTON1_PIN, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);
  timer.every(3, [](void *arg) -> bool {
      unsigned long currentMillis = millis();
      button1.process(currentMillis);
      return true;
  });

  // Setup Climate Display
  climateDisplay.setRoomHumidity(Room::Kitchen, 50);

  // Setup rotary encoder
  pinMode(ROTARY_PIN_A, INPUT_PULLUP);
  pinMode(ROTARY_PIN_B, INPUT_PULLUP);
  encoder.begin();
  timer.every(10, [](void *arg) -> bool {
    int16_t currentValue;
    NewEncoder::EncoderState currentEncoderState;

      if(encoder.getState(currentEncoderState)) {
        Serial.println("Encoder: ");
        currentValue = currentEncoderState.currentValue;

        // Handle big jumps
        if(currentValue > (prevEncoderValue + 1)) {
            currentValue = prevEncoderValue + 1;
        } else if(currentValue < (prevEncoderValue - 1)) {
            currentValue = prevEncoderValue - 1;
        }

        prevEncoderValue = currentValue;
        Serial.println(currentValue);
      }     
      return true;
    });

    // Ultrasonic Distance Sensor
    timer.every(1000, [](void *arg) -> bool {
      Serial.println(hc.dist());
      return true;
    });

}

void loop(void) {
  delay(1);
  climateDisplay.loop();
  timer.tick();
}
