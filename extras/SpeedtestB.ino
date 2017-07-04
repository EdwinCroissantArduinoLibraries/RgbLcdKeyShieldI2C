#include "Arduino.h"
#include <Wire.h>
#include <RgbLcdKeyShield.h>

RgbLcdKeyShield lcd;

void setup() {
//  Debugging output
	Serial.begin(9600);
	Wire.begin();
	// set the wire bus to 400 kHz
	Wire.setClock(400000L);
	lcd.begin();

	// Print a message to the LCD. We track how long it takes.
	uint32_t time = micros();
	uint8_t n = lcd.print("Robotdyn test!");
	time = micros() - time;
//	Serial.println("Adafruit_RGBLCDShield.h");
	Serial.println("------RgbLcdKeyShield.h");
	Serial.println("");
	Serial.println("Wire bus at at 400 kHz");
	Serial.println("");
	Serial.print(n);
	Serial.print(" characters in ");
	Serial.print(time);
	Serial.print(F(" us = "));
	Serial.print(n * 1000000 / time);
	Serial.println(F(" char/s"));

	// now for one character
	lcd.setCursor(0, 1);
	time = micros();
	n = lcd.print('B');
	time = micros() - time;

	Serial.print(n);
	Serial.print(" characters in ");
	Serial.print(time);
	Serial.print(F(" us = "));
	Serial.print(n * 1000000 / time);
	Serial.println(F(" char/s"));
	Serial.println("");

	lcd.setColor(RgbLcdKeyShield::clWhite);
}

void loop() {

}
