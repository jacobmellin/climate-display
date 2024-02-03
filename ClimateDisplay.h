#include <map>
#include <FastLED.h>

enum class Room
{
    Kitchen,
    Bathroom,
    Bedroom,
    Corridor
};

class ClimateDisplay {
    private:
        struct RoomData {
            Room room;
            float temperature;
            float humidity;
            bool windowOpen;
            int ledIndex;
        };
        std::map<Room, RoomData> rooms; 
        CRGB* leds;

    public:
        ClimateDisplay(CRGB* leds) {
            this->leds = leds;

            rooms[Room::Kitchen] = RoomData();
            rooms[Room::Kitchen].room = Room::Kitchen;
            rooms[Room::Kitchen].ledIndex = 0;

            rooms[Room::Bathroom] = RoomData();
            rooms[Room::Bathroom].room = Room::Bathroom;
            rooms[Room::Bathroom].ledIndex = 1;

            rooms[Room::Bedroom] = RoomData();
            rooms[Room::Bedroom].room = Room::Bedroom;
            rooms[Room::Bedroom].ledIndex = 2;

            rooms[Room::Corridor] = RoomData(); 
            rooms[Room::Corridor].room = Room::Corridor;
            rooms[Room::Corridor].ledIndex = 3;
        };


        void setRoomTemperature(Room room, float temperature) {
            rooms[room].temperature = temperature;
        };

        void setRoomHumidity(Room room, float humidity) {
            rooms[room].humidity = humidity;
        };

        void setRoomWindowOpen(Room room, bool open) {
            rooms[room].windowOpen = open;
        };

        void displayHumidityOnLEDs () {
            for (auto room : rooms) {
                if (room.second.humidity >= 57) {
                    leds[room.second.ledIndex] = CRGB::Red;
                } else if (room.second.humidity >= 54) {
                    leds[room.second.ledIndex] = CRGB::Orange;
                } else {
                    leds[room.second.ledIndex] = CRGB::Green;
                }
            }

            FastLED.show();
        };
};
