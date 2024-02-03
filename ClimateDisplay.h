#include <map>
#include <FastLED.h>

#define LED_FADE_DURATION_MS 1000

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
    Humidity
};

class ClimateDisplay {
    private:
        struct RoomData {
            Room room;
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
        bool isStandby = false;
        float humidityWarningThreshold = 53;
        float humiditiyCriticalThreshold = 57;

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
        ClimateDisplay(CRGB* leds) {
            this->leds = leds;

            displayLEDs[LED::Kitchen] = LEDData();
            displayLEDs[LED::Kitchen].ledIndex = 0;
            displayLEDs[LED::Kitchen].color = CRGB::Black;

            displayLEDs[LED::Bathroom] = LEDData();
            displayLEDs[LED::Bathroom].ledIndex = 1;
            displayLEDs[LED::Bathroom].color = CRGB::Black;

            displayLEDs[LED::Bedroom] = LEDData();
            displayLEDs[LED::Bedroom].ledIndex = 2;
            displayLEDs[LED::Bedroom].color = CRGB::Black;

            displayLEDs[LED::Corridor] = LEDData();
            displayLEDs[LED::Corridor].ledIndex = 3;
            displayLEDs[LED::Corridor].color = CRGB::Black;

            displayLEDs[LED::Window] = LEDData();
            displayLEDs[LED::Window].ledIndex = 4;
            displayLEDs[LED::Window].color = CRGB::Black;

            displayLEDs[LED::Humidity] = LEDData();
            displayLEDs[LED::Humidity].ledIndex = 5;
            displayLEDs[LED::Humidity].color = CRGB::Black;

            rooms[Room::Kitchen].room = Room::Kitchen;
            rooms[Room::Kitchen].led = LED::Kitchen;

            rooms[Room::Bathroom] = RoomData();
            rooms[Room::Bathroom].room = Room::Bathroom;
            rooms[Room::Bathroom].led = LED::Bathroom;

            rooms[Room::Bedroom] = RoomData();
            rooms[Room::Bedroom].room = Room::Bedroom;
            rooms[Room::Bedroom].led = LED::Bedroom;

            rooms[Room::Corridor] = RoomData(); 
            rooms[Room::Corridor].room = Room::Corridor;
            rooms[Room::Corridor].led = LED::Corridor;
        };


        void setRoomTemperature(Room room, float temperature) {
            rooms[room].temperature = temperature;
        };

        void setRoomHumidity(Room room, float humidity) {
            rooms[room].humidity = humidity;
            updateLEDs();
        };

        void setRoomWindowOpen(Room room, bool open) {
            rooms[room].windowOpen = open;
        };

        void setStandby(bool standby) {
            isStandby = standby;
        };

        void setHumidityWarningThreshold(float threshold) {
            humidityWarningThreshold = threshold;
        };

        void setHumidityCriticalThreshold(float threshold) {
            humiditiyCriticalThreshold = threshold; 
        };

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
            
        };

        void updateLEDs() {
            for (auto room : rooms) {
                if (room.second.humidity >= humiditiyCriticalThreshold) {
                    fadeDisplayLED(room.second.led, CRGB::Red);
                } else if (room.second.humidity >= humidityWarningThreshold) {
                    fadeDisplayLED(room.second.led, CRGB::Yellow);
                } else {
                    if (!isStandby) {
                        fadeDisplayLED(room.second.led, CRGB::Green);
                    } else {
                        fadeDisplayLED(room.second.led, CRGB::Black);
                    }
                }
            }
        };

        void loop() {
            handleLEDFade();
        };
};
