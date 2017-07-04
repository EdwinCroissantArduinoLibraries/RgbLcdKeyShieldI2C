#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#define WHITE 0x7

void setup() {
//  Debugging output
	Serial.begin(9600);
	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	Wire.setClock(400000L);

	// Print a message to the LCD. We track how long it takes since
	// this library has been optimized a bit and we're proud of it :)
	// Print a message to the LCD. We track how long it takes.
	uint32_t time = micros();
	uint8_t n = lcd.print("Robotdyn test!");
	time = micros() - time;
	Serial.println("Adafruit_RGBLCDShield.h");
//	Serial.println("------RgbLcdKeyShield.h");
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
	n = lcd.print('A');
	time = micros() - time;

	Serial.print(n);
	Serial.print(" characters in ");
	Serial.print(time);
	Serial.print(F(" us = "));
	Serial.print(n * 1000000 / time);
	Serial.println(F(" char/s"));
	Serial.println("");

	lcd.setBacklight(WHITE);
}

void loop() {

}

