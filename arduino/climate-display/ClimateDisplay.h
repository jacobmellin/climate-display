#include <map>
#include <FastLED.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <string>
#include <iomanip>
#include <sstream>

#define LED_FADE_DURATION_MS 1000
#define STANDBY_TIMEOUT_MS 10000

enum class Room
{
    Kitchen,
    Bathroom,
    Bedroom,
    Corridor
};

enum class LED
{
    Kitchen,
    Bathroom,
    Bedroom,
    Corridor,
    Window,
    Humidity,
    Temperature
};

class ClimateDisplay {
    private:
        struct RoomData {
            Room room;
            const char* name;
            float temperature;
            float humidity;
            bool windowOpen;
            LED led;
        };

        struct LEDData {
            int ledIndex;
            CRGB color;
            CRGB fromColor;
            CRGB toColor;
            int fadeStart;
            float fadeProgress;
            bool isFading;
        };

        std::map<Room, RoomData> rooms; 
        std::map<LED, LEDData> displayLEDs;

        CRGB* leds;
        Adafruit_SH1106G* display;
        bool isStandby = false;
        float humidityWarningThreshold = 55;
        float humiditiyCriticalThreshold = 60;
        float temperatureCriticalThreshold = 19;
        float userDistance = 0;
        float userDistanceThreshold = 40;

        float startWakeTime = 0;

        void replaceSpecialChars(const char* text) {
            for (int i = 0; i < sizeof(text); i++) {
            }
        }

        void handleLEDFade() {
            for(auto& led : displayLEDs) {
                if(led.second.isFading) {
                    led.second.fadeProgress = 
                        float(millis() - led.second.fadeStart) / LED_FADE_DURATION_MS;
                    if(led.second.fadeProgress >= 1) {
                        led.second.isFading = false;
                        led.second.color = led.second.toColor;
                    } else {
                        led.second.color = blend(
                                led.second.fromColor,
                                led.second.toColor,
                                led.second.fadeProgress*255);
                    }
                    leds[led.second.ledIndex] = led.second.color;
                    FastLED.show();
                }
            }
        }

    public:
        ClimateDisplay(CRGB* leds, Adafruit_SH1106G* display) {
            this->leds = leds;
            this->display = display;

            displayLEDs[LED::Kitchen] = LEDData();
            displayLEDs[LED::Kitchen].ledIndex = 3;
            displayLEDs[LED::Kitchen].color = CRGB::Black;

            displayLEDs[LED::Bathroom] = LEDData();
            displayLEDs[LED::Bathroom].ledIndex = 4;
            displayLEDs[LED::Bathroom].color = CRGB::Black;


            displayLEDs[LED::Corridor] = LEDData();
            displayLEDs[LED::Corridor].ledIndex = 5;
            displayLEDs[LED::Corridor].color = CRGB::Black;

            displayLEDs[LED::Bedroom] = LEDData();
            displayLEDs[LED::Bedroom].ledIndex = 6;
            displayLEDs[LED::Bedroom].color = CRGB::Black;

            displayLEDs[LED::Window] = LEDData();
            displayLEDs[LED::Window].ledIndex = 2;
            displayLEDs[LED::Window].color = CRGB::Black;

            displayLEDs[LED::Humidity] = LEDData();
            displayLEDs[LED::Humidity].ledIndex = 0;
            displayLEDs[LED::Humidity].color = CRGB::Black;
            
            displayLEDs[LED::Temperature] = LEDData();
            displayLEDs[LED::Temperature].ledIndex = 0;
            displayLEDs[LED::Temperature].color = CRGB::Black;

            rooms[Room::Kitchen] = RoomData();
            rooms[Room::Kitchen].room = Room::Kitchen;
            rooms[Room::Kitchen].led = LED::Kitchen;
            rooms[Room::Kitchen].name = "K\201che";

            rooms[Room::Bathroom] = RoomData();
            rooms[Room::Bathroom].room = Room::Bathroom;
            rooms[Room::Bathroom].led = LED::Bathroom;
            rooms[Room::Bathroom].name = "Bad";

            rooms[Room::Bedroom] = RoomData();
            rooms[Room::Bedroom].room = Room::Bedroom;
            rooms[Room::Bedroom].led = LED::Bedroom;
            rooms[Room::Bedroom].name = "Zimmer r.";

            rooms[Room::Corridor] = RoomData(); 
            rooms[Room::Corridor].room = Room::Corridor;
            rooms[Room::Corridor].led = LED::Corridor;
            rooms[Room::Corridor].name = "Flur";
        };


        void setRoomTemperature(Room room, float temperature) {
            rooms[room].temperature = temperature;
            updateLEDs();
            drawScreen();
        };

        void setRoomHumidity(Room room, float humidity) {
            rooms[room].humidity = humidity;
            updateLEDs();
            drawScreen();
        };

        void setRoomWindowOpen(Room room, bool open) {
            rooms[room].windowOpen = open;
        };

        void setStandby(bool standby) {
            isStandby = standby;
            if(!isStandby) {
                // Display has been woken up     
               startWakeTime = millis();
            }
            updateLEDs();
            drawScreen();
        };

        void setHumidityWarningThreshold(float threshold) {
            humidityWarningThreshold = threshold;
        };

        void setHumidityCriticalThreshold(float threshold) {
            humiditiyCriticalThreshold = threshold; 
        };

        void setUserDistance(float distance) {
            userDistance = distance;
        }

        void fadeDisplayLED(LED led, CRGB target) {
            if(!displayLEDs[led].isFading) {
                if(displayLEDs[led].color == target) {
                    return;
                }
                displayLEDs[led].fromColor = displayLEDs[led].color;
                displayLEDs[led].toColor = target;
                displayLEDs[led].isFading = true;
                displayLEDs[led].fadeProgress = 0;
                displayLEDs[led].fadeStart = millis();
            } else {
                if(displayLEDs[led].toColor != target) {
                    displayLEDs[led].toColor = target;
                    displayLEDs[led].fromColor = displayLEDs[led].color;
                    displayLEDs[led].fadeProgress = 0;
                    displayLEDs[led].fadeStart = millis();
                }
            }
        };

        void drawScreen() {
          display->clearDisplay();

          if(isStandby) {
            display->display();
            return;
          }

          display->setTextSize(1);
          display->setTextColor(SH110X_WHITE);
          display->setCursor(0,1);
          display->cp437(true);

          bool first = true;

          for (auto room : rooms) {
            if(first) {
                first = false;
            } else {
                display->print("\n");
            }

            std::stringstream label;
            label << std::left << std::setw(10) << room.second.name;
            display->print(label.str().c_str());

            display->print("  ");

            std::stringstream humidity;
            humidity << std::right << std::fixed << std::setw(5) << std::setprecision(1) << room.second.humidity;
            display->print(humidity.str().c_str());
            display->print(" %RH");

            display->print("\n            ");


            std::stringstream temperature;
            temperature << std::fixed << std::setw(5) << std::setprecision(1) << room.second.temperature;
            display->print(temperature.str().c_str());

            display->print("  ");
            display->write(248);
            display->print("C");
          }

          display->display();
        };

        void updateLEDs() {
            int numCriticalHumidity = 0;
            int numCriticalTemperature = 0;

            for (auto room : rooms) {
                if (room.second.humidity >= humiditiyCriticalThreshold) {
                    fadeDisplayLED(room.second.led, CRGB::Red);
                    fadeDisplayLED(LED::Humidity, CRGB::Red);
                    numCriticalHumidity++;
                } else if (room.second.humidity >= humidityWarningThreshold) {
                    if(!isStandby) {
                        fadeDisplayLED(room.second.led, CRGB::Yellow);
                    } else {
                        fadeDisplayLED(room.second.led, CRGB::Black);
                    }
                } else {
                    if (!isStandby) {
                        fadeDisplayLED(room.second.led, CRGB::Green);
                    } else {
                        fadeDisplayLED(room.second.led, CRGB::Black);
                    }
                }

                if(room.second.temperature <= temperatureCriticalThreshold) {
                    fadeDisplayLED(LED::Temperature, CRGB::Red);
                }
            }

            if(numCriticalHumidity < 1) {
                fadeDisplayLED(LED::Humidity, CRGB::Black);
            }

            if(numCriticalTemperature < 1) {
                fadeDisplayLED(LED::Temperature, CRGB::Black);
            }
        };

        void loop() {
            handleLEDFade();

            if(!isStandby) {
                if(millis() - startWakeTime > STANDBY_TIMEOUT_MS) {
                    setStandby(true);
                }
            }

            if(userDistance < userDistanceThreshold) {
                setStandby(false);
            }
        };
};
