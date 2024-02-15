# 3D Printed Room Climate Display/Controller based on ESP32/Arduino

Let's [Stoßlüft](https://www.youtube.com/watch?v=y79x2q360W8)!

TODO: Images/Video

The DIY Climate Display/Controller thing can do the following:

- Shows Temperature and Relative Humidity information for 4 different rooms at once!
- Can be controlled via MQTT/WiFi, so you can for example setup Home Assistant to set Temperature/Humidity Information
- Will light colored LEDs to tell you when Humidity/Temperature in a room rises above a certain threshold (e. g. Yellow: 57%, Red: 60%).
- Notifies you when you forget to close the windows
- Allows you to set the temperature for your smart thermostats via MQTT

## Components

| Qty. | Component Type                     | Specific Component used | Est. Price (USD) | Est. Price (€) | Example Shop link (DE)                                     |   |
|------|------------------------------------|-------------------------|------------------|----------------|------------------------------------------------------------|---|
| 2m   | Wire in 3 different colors         |                         |                  |                |                                                            |   |
|  1x  | ESP32 Development Board            | ESP32 Lolin32 Lite      |                  | € 8,99         | https://www.az-delivery.de/en/products/esp32-lolin-lolin32 |   |
|  1x  | Ultrasonic Distance Sensor         | HC-SR04                 |                  |                |                                                            |   |
|  1x  | OLED Display                       | SH1106 (128x64)         |                  |                |                                                            |   |
|  7x  | Addressible RGB LED                | WSB2812B                |                  |                |                                                            |   |
|  1x  | Rotary Encoder                     |                         |                  |                |                                                            |   |
|  2x  | Push Button (optional)             |                         |                  |                |                                                            |   |
| 2x   | 1K Metal Film resistor             |                         |                  |                |                                                            |   |
| 1x   | USB Power Supply / Micro USB Cable |                         |                  |                |                                                            |   |

## Printing & Assembly

TODO

## Configuring & Programming

TODO
