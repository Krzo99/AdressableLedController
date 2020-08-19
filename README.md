# AdressableLedController

This is a controller for adressable leds. It's made using a custom PCB for ESP32 (Overkill, yes). Leds are controlled over Wi-Fi, and with [sound detector module from SparkFun](https://www.sparkfun.com/products/12642).  Should be able to upload using usb port or [FTD232RL module](https://www.banggood.com/Geekcreit-FT232RL-FTDI-USB-To-TTL-Serial-Converter-Adapter-Module-Geekcreit-for-Arduino-products-that-work-with-official-Arduino-boards-p-917226.html?cur_warehouse=CN).

# Known issues in this version:

- PCB should be directly connected from TX to RX, without adapter.
- Analog inputs, or at least Envelope, should be connected to a port with ADC1. (ADC2 does not work with WiFi ON!!).
