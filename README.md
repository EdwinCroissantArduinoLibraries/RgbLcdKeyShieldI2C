# This is a library for the Adafruit RGB LCD Shield Kit and the RobotDyn LCD RGB 16x2 + keypad + Buzzer Shield for Arduino using the [I2C library from Wayne Truchsess](https://github.com/rambo/I2C)
This library is written from scratch and makes full use of the 8 bit capability of the MCP23017 to send multiple packets to the HD44780 in one transmission, minimizing the overhead of the two wire address and register commands. At a wire bus speed of 400 kHz throughputs of more than 5200 characters per second for single characters and up to 8100 characters per second for strings are reached. 

This is nearly 11 times as the fast for single characters and nearly 17 times as fast for strings compared to the Adafruit library at the same bus speed.

It can print to the lcd and load special characters into the lcd directly from program memory with the printP and createCharP command.

The buttons have callback functions for short press, long press, repeating and there is a static callback for two buttons pressed at the same time. 
