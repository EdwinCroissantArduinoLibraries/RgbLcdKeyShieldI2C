#include "Arduino.h"
#include <I2C.h>
#include <RgbLcdKeyShieldI2C.h>

// make an instance of RgbLcdKeyShield
RgbLcdKeyShieldI2C lcd;

/*
 * Strings in program memory can be displayed with the printP function
 */
const char msgStart[] PROGMEM = "Press select";
const char msgGreeting[] PROGMEM = "Hello world, how are you today?";

/*
 * Special characters in program memory can be loaded with the
 * createCharP function Note that the characters are 8 bit high,
 * the last byte in the array is displayed at the cursor line
 */

const uint8_t PROGMEM smiley[] = {
B00000,
B10001,
B00100,
B00100,
B10001,
B01110,
B00000,
B00000 };

/*
 * The buzzer is connected to pin 2 (the board says pin 3 :( )
 */

enum pins {
	buzzerPin = 2
};

uint32_t time;
uint32_t n;
bool blinkOn, cursorOn, autoscroll;

void setup() {
	I2c.begin();
//	I2c.pullup(0);
	// set the wire bus to 400 kHz
	I2c.setSpeed(1);
	I2c.timeOut(10);
	lcd.begin();
	lcd.createCharP(0,smiley);
	startDemo();
}

void loop() {
	  lcd.readKeys();
}

void startDemo() {
	lcd.clear();
	lcd.clearKeys();
	lcd.setColor(RgbLcdKeyShieldI2C::clWhite);
	lcd.setCursor(4,0);
	lcd.print(F("To start"));
	lcd.setCursor(2,1);
	lcd.print(F("Press select"));
	lcd.keySelect.onShortPress = backlightDemo;
	lcd.keySelect.onLongPress = cursorDemo;
}

void backlightDemo() {
	lcd.clear();
	lcd.clearKeys();
	lcd.setCursor(4,0);
	lcd.print(F("RobotDyn"));
	lcd.setCursor(1,1);
	lcd.print(F("Backlight test"));

	for (int i = 0; i < 8; ++i) {
		lcd.setColor(RgbLcdKeyShieldI2C::colors(i));
		delay(500);
	}

	lcd.keyUp.onShortPress = setBlack;
	lcd.keyUp.onLongPress = setRed;
	lcd.keyRight.onShortPress = setGreen;
	lcd.keyRight.onLongPress = setYellow;
	lcd.keyDown.onShortPress = setBlue;
	lcd.keyDown.onLongPress = setViolet;
	lcd.keyLeft.onShortPress = setTeal;
	lcd.keyLeft.onLongPress = setWhite;
	lcd.keyUp.onShortPress = setBlack;

	lcd.keySelect.onShortPress =  speedDemo;
	lcd.keySelect.onLongPress = startDemo;
}

void speedDemo() {
	lcd.clear();
	lcd.clearKeys();

	time = micros();
	n = lcd.printP(msgGreeting);
	time = micros() - time;

	lcd.setCursor(0,1);
	lcd.print(n);
	lcd.print(" ch in ");
	lcd.print(time);
	lcd.print(' ');
	lcd.print(char(0xE4));
	lcd.print(F("s = "));
	lcd.print(n * 1000000 / time);
	lcd.print(F(" char/s "));
	lcd.print(char(0));

	for (int i = 0; i < 40; i++) {
		lcd.scrollDisplayLeft();
		delay(300);
	}
	lcd.keyLeft.onShortPress =  moveLeft;;
	lcd.keyLeft.onRepPress = moveLeft;
	lcd.keyRight.onShortPress = moveRight;
	lcd.keyRight.onRepPress = moveRight;

	lcd.keySelect.onShortPress = soundDemo;
	lcd.keySelect.onLongPress = backlightDemo;
}

void soundDemo() {
	lcd.clear();
	lcd.clearKeys();
	lcd.setColor(RgbLcdKeyShieldI2C::clWhite);
	lcd.setCursor(3,0);
	lcd.print(F("Beeper demo"));
	delay(1000);
	lcd.clear();
	lcd.setCursor(0,1);
	lcd.print(F("Press down long"));
	lcd.setColor(RgbLcdKeyShieldI2C::clWhite);
	lcd.keyDown.onRepPressCount = beepLadder;
	lcd.keySelect.onShortPress = twoKeyDemo;
	lcd.keySelect.onLongPress = speedDemo;
}

void twoKeyDemo() {
	lcd.clear();
	lcd.clearKeys();
	lcd.setColor(RgbLcdKeyShieldI2C::clWhite);
	lcd.setCursor(1,0);
	lcd.print(F("Press two keys"));
	lcd.keySelect.onTwoPress = twoKey;
	lcd.keyDown.onShortPress = clearRow1;
	lcd.keyUp.onShortPress = clearRow1;
	lcd.keyLeft.onShortPress = clearRow1;
	lcd.keyRight.onShortPress = clearRow1;
	lcd.keySelect.onShortPress = displayDemo;
	lcd.keySelect.onLongPress = soundDemo;
}

void displayDemo() {
	lcd.clear();
	lcd.clearKeys();
	lcd.setColor(RgbLcdKeyShieldI2C::clWhite);
	lcd.setCursor(2,0);
	lcd.print(F("Display demo"));
	delay(1000);
	lcd.clear();
	lcd.setCursor(1,0);

	lcd.keyLeft.onShortPress =  moveLeft;;
	lcd.keyLeft.onRepPress = moveLeft;
	lcd.keyRight.onShortPress = moveRight;
	lcd.keyRight.onRepPress = moveRight;
	lcd.keyDown.onShortPress = clearLcd;
	lcd.keyDown.onRepPressCount = charOut;
	lcd.keyUp.onShortPress = leftToRight;
	lcd.keyUp.onLongPress = rightToLeft;
	lcd.keySelect.onShortPress = cursorDemo;
	lcd.keySelect.onLongPress = twoKeyDemo;
}

void cursorDemo() {
	lcd.clear();
	lcd.clearKeys();
	lcd.setColor(RgbLcdKeyShieldI2C::clWhite);
	lcd.setCursor(2,0);
	lcd.print(F("Cursor demo"));
	delay(1000);
	lcd.clear();
	lcd.keyLeft.onShortPress =  moveCursorLeft;;
	lcd.keyLeft.onRepPress = moveCursorLeft;
	lcd.keyRight.onShortPress = moveCursorRight;
	lcd.keyRight.onRepPress = moveCursorRight;
	lcd.keyUp.onShortPress = toggleBlink;
	lcd.keyUp.onLongPress = toggleCursor;
	lcd.keySelect.onShortPress = startDemo;
	lcd.keySelect.onLongPress = displayDemo;
	lcd.keyDown.onShortPress = toggleAutoscroll;
	lcd.keyDown.onRepPressCount = charOut;
	lcd.keySelect.onTwoPress = twoKeyClearDisplay;
}

void toggleAutoscroll() {
	autoscroll = !autoscroll;
	if (autoscroll) lcd.autoscroll();
	else lcd.noAutoscroll();
}

void toggleBlink() {
	blinkOn = !blinkOn;
	if (blinkOn) lcd.blink();
	else lcd.noBlink();
}

void toggleCursor () {
	cursorOn = !cursorOn;
	if (cursorOn) lcd.cursor();
	else lcd.noCursor();
}

void setBlack() {
	lcd.setColor(RgbLcdKeyShieldI2C::clBlack);
}

void setRed() {
	lcd.setColor(RgbLcdKeyShieldI2C::clRed);
}

void setGreen() {
	lcd.setColor(RgbLcdKeyShieldI2C::clGreen);
}

void setYellow() {
	lcd.setColor(RgbLcdKeyShieldI2C::clYellow);
}

void setBlue() {
	lcd.setColor(RgbLcdKeyShieldI2C::clBlue);
}

void setViolet() {
	lcd.setColor(RgbLcdKeyShieldI2C::clViolet);
}

void setTeal() {
	lcd.setColor(RgbLcdKeyShieldI2C::clTeal);
}

void setWhite() {
	lcd.setColor(RgbLcdKeyShieldI2C::clWhite);
}

void moveRight() {
	lcd.scrollDisplayRight();
}

void moveLeft() {
	lcd.scrollDisplayLeft();
}

void clearLcd() {
	lcd.clear();
}

void moveCursorRight() {
	lcd.moveCursorRight();
}

void moveCursorLeft() {
	lcd.moveCursorLeft();
}

void autoScroll() {
	lcd.autoscroll();
}

void noAutoScroll() {
	lcd.noAutoscroll();
}

void beepLadder (uint16_t count) {
	static const uint16_t tones[] PROGMEM = {
	440, 494, 523, 587, 659, 698, 784};
	count %= 7;
	tone (buzzerPin,pgm_read_word(&tones[count]),100);
	lcd.setCursor(6, 0);
	lcd.print(pgm_read_word(&tones[count]), DEC);
	lcd.print(F(" Hz"));
}

void printKeyName(const SimpleKeyHandler* key) {
	if ( key == &lcd.keyDown) lcd.print(F("Down"));
		else if (key == &lcd.keyLeft) lcd.print(F("Left"));
			else if (key == &lcd.keyRight) lcd.print(F("Right"));
				else if (key == &lcd.keySelect) lcd.print(F("Select"));
					else if (key == &lcd.keyUp) lcd.print(F("Up"));
}

void twoKey(const SimpleKeyHandler* senderKey, const SimpleKeyHandler* otherKey) {
	clearRow1();
	lcd.setCursor(0, 1);
	printKeyName(senderKey);
	lcd.setCursor(8, 1);
	printKeyName(otherKey);
}

void twoKeyClearDisplay(const SimpleKeyHandler* senderKey,
		const SimpleKeyHandler* otherKey) {
	if ((senderKey == &lcd.keyLeft || otherKey == &lcd.keyLeft)
			&& (senderKey == &lcd.keySelect || otherKey == &lcd.keySelect))
		lcd.clear();
}

void clearRow1() {
	lcd.setCursor(0, 1);
	for (int i = 0; i < 16; ++i) {
		lcd.print(' ');
	}
}

void charOut(uint16_t count) {
	static uint8_t c;
	// reset c when key is pressed
	if (count == 0)
		c = 0;
	for (int i = 0; i < 4; ++i) {
		// skip duplicate special characters
		if (c == 7)
			c = 32;
		// skip blank characters
		if (c == 128)
			c = 160;
		lcd.print(char(c));
		c++;
	}
}

void leftToRight() {
	lcd.clear();
	lcd.setCursor(1,0);
	lcd.leftToRight();
	lcd.print(F("Left to Right"));
}

void rightToLeft() {
	lcd.clear();
	lcd.setCursor(13,0);
	lcd.rightToLeft();
	lcd.print(F("Right to left"));
}


