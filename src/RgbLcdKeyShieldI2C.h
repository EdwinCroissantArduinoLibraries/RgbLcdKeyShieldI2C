/*
 * This is a library for the Adafruit RGB LCD Shield Kit and the
 * RobotDyn LCD RGB 16x2 + keypad + Buzzer Shield for Arduino
 *
 * Uses the I2C library from Wayne Truchsess
 *
 * Copyright (C) 2017 Edwin Croissant
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See the README.md file for additional information.
 */
/*
 * version
 * 0.0.3	2021/03/08 introduced inverted backlight option
 * 0.0.2	2017/07/11 introduced read and getCursor for the lcd
 * 0.0.1	2017/07/04 initial version
 */

#ifndef RgbLcdKeyShieldI2C_H
#define RgbLcdKeyShieldI2C_H

#include "Arduino.h"
#include "I2C.h"

class SimpleKeyHandler {
public:
	SimpleKeyHandler();
	void read(bool keyState);
	void clear();
	bool isPressed();
	// Called when the key is released before the long press time expired.
	void (*onShortPress)();
	// Called when the long press time expired
	void (*onLongPress)();
	// Repeatedly called when the long press time expired
	void (*onRepPress)();
	// Repeatedly called when the long press time expired, the count is passed as an argument
	void (*onRepPressCount)(uint16_t count);
	// Called when two keys are pressed at the same time
	static void (*onTwoPress)(const SimpleKeyHandler* senderKey, const SimpleKeyHandler* otherKey);
private:
	enum lastKeyState {
		keyOff, keyToOn, keyOn, keyToOff
	} _previousState;
	enum keyTime {
		debounce = 50,
		longPress = 500,
		repeatInterval = 250
	};
	uint32_t _nextValidRead;
	bool _allowEvents;
	static uint16_t _count;
	static SimpleKeyHandler* _activeKey;
	static SimpleKeyHandler* _otherKey;
};

class RgbLcdKeyShieldI2C: public Print {
public:
	enum colors: uint8_t {
		clBlack = 0,
		clRed = 1,
		clGreen = 2,
		clYellow = 3,
		clBlue = 4,
		clViolet = 5,
		clTeal = 6,
		clWhite = 7
	};

	using Print::write; // pull in write(str) and write(buf, size) from Print

	RgbLcdKeyShieldI2C(bool invertedBacklight = false);

	void begin();
	void clear();
	void home();
	void setCursor(uint8_t col, uint8_t row);
	void setColor(colors color);
	void display();
	void noDisplay();
	void blink();
	void noBlink();
	void cursor();
	void noCursor();
	void scrollDisplayRight();
	void scrollDisplayLeft();
	void leftToRight();
	void rightToLeft();
	void moveCursorRight();
	void moveCursorLeft();
	void autoscroll();
	void noAutoscroll();
	void createChar(uint8_t location, const uint8_t *charmap);
#ifdef __AVR__
	void createCharP(uint8_t location, const uint8_t *charmap);
	size_t printP(const char str[]);
	size_t writeP(const uint8_t *buffer, size_t size);
#endif // __AVR__
	virtual size_t write(uint8_t c);
	size_t write(const uint8_t *buffer, size_t size) override;
	uint8_t read();
	size_t read(uint8_t *buffer, size_t size);
	uint8_t getCursor();

	void readKeys();
	void clearKeys();
	SimpleKeyHandler keyLeft;
	SimpleKeyHandler keyRight;
	SimpleKeyHandler keyUp;
	SimpleKeyHandler keyDown;
	SimpleKeyHandler keySelect;
private:
	// 8 bit mode MCP23017 register addresses
	enum MCP23017 {
		I2Caddr = 0x20,
		IOCON = 0x0b,
		IODIRA = 0x00,
		IPOLA = 0x01,
		IODIRB = 0x10,
		GPIOA = 0x09,
		GPIOB = 0x19,
		GPPUA = 0x06
	};

	// HD44780 constants
	enum HD44780 {
		// commands
		clearDisplay = 0x01,
		returnHome = 0x02,
		entryModeSet = 0x04,
		displayControl = 0x08,
		curOrDispShift = 0x10,
		functionSet = 0x20,
		setCgRamAdr = 0x40,
		setDdRamAdr = 0x80,
		// flags for entry mode set
		autoShiftFlag = 0x01,
		left2RightFlag = 0x02, // 1 = left to right, 0 = right to left,
		// flags for display on/off control
		displayOnFlag = 0x04,
		cursorOnFlag = 0x02,
		blinkOnFlag = 0x01,
		// flags for cursor/display shift
		displayShiftFlag = 0x08, // 1 = display, 0 is cursor
		shiftRightFlag = 0x04, // 1 = right, 0 = left
		// flags for function set
		bitMode8Flag = 0x10, // 8 bit = 1, 4 bit = 0
		lineMode2Flag = 0x08, // 2 line = 1, 1 line = 0
		dots5x10Flag = 0x04 // 5x10 dots = 1, 5x8 dots = 0
	};

	// shadow registers  MCP23017 GPIOA and GPIOB
	int8_t _shadowGPIOA;
	int8_t _shadowGPIOB;

	// shadow registers HD44780 display control and Entry Mode Set
	int8_t _shadowDisplayControl;
	int8_t _shadowEntryModeSet;

	// translation table from nibble to pin
	static const uint8_t _nibbleToPin[16];

	bool _invertedBacklight;

	void _lcdWrite4(uint8_t value, bool lcdInstruction);
	inline void _lcdWrite8(uint8_t value, bool lcdInstruction);
	void _lcdTransmit(uint8_t value, bool lcdInstruction);
	void _prepareRead(bool lcdInstruction);
	uint8_t _lcdRead4();
	inline uint8_t _lcdRead8();
	inline void _cleanupRead();
};



#endif //  RgbLcdKeyShieldI2C_H

