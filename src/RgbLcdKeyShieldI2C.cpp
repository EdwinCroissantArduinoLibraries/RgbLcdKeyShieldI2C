/*
 * This is a library for the Adafruit RGB LCD Shield Kit and the
 * RobotDyn LCD RGB 16x2 + keypad + Buzzer Shield for Arduino
 *
 * Uses the Wire and SimpleKeyHandler library
 *
 * Copyright (C) 2017 Edwin Croissant
 *
 *  This program is free software: you can redistribute it and/or modify
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

#include <RgbLcdKeyShieldI2C.h>

//--------------------------------SimpleKeyHandler----------------------------
/*
 * These variables and function pointer are defined as static as they
 * are common for all instances of this class
 */
uint16_t SimpleKeyHandler::_count = 0;
SimpleKeyHandler* SimpleKeyHandler::_activeKey = nullptr;
SimpleKeyHandler* SimpleKeyHandler::_otherKey = nullptr;
void (*SimpleKeyHandler::onTwoPress)(const SimpleKeyHandler* senderKey,
		const SimpleKeyHandler* otherKey) = nullptr;

SimpleKeyHandler::SimpleKeyHandler() {
	clear();
	_nextValidRead = 0;
	_previousState = keyOff;
	_allowEvents = false;
}
/*
 * Clears all the callback pointers;
 */
void SimpleKeyHandler::clear() {
	onShortPress = nullptr;
	onLongPress = nullptr;
	onRepPress = nullptr;
	onRepPressCount = nullptr;
	onTwoPress = nullptr;
}

/*
 * To be placed in the main loop. Expect TRUE if a key is pressed.
 */
void SimpleKeyHandler::read(bool keyState) {
	switch (_previousState) {
	case keyOff:
		if (keyState) {
			// when on advance to the next state
			_previousState = keyToOn;
			_nextValidRead = millis() + debounce;
		}
		break;
	case keyToOn:
		// ignore the key until debounce time expired
		if (millis() >= _nextValidRead) {
			if (keyState) {
				// when still on advance to the next state
				_previousState = keyOn;
				_nextValidRead = millis() + longPress;
				// disable the other keys
				if (!_activeKey)
					_activeKey = this;
				// try to claim the other key
				else if (!_otherKey)
						_otherKey = this;
				_allowEvents = (_activeKey == this);
			} else
				// otherwise it was a glitch
				_previousState = keyOff;
		}
		break;
	case keyOn:
		if (!keyState) {
			// when off advance to the next state
			_previousState = keyToOff;
			_nextValidRead = millis() + debounce;
		} else {
			// callback after long press and repeat after the repeat interval
			if (millis() >= _nextValidRead) {
				_nextValidRead = millis() + repeatInterval;
				// prevent events when disabled
				if ((_allowEvents)) {
					if (onLongPress && _count == 0)
						onLongPress();
					if (onRepPressCount)
						onRepPressCount(_count);
					if (onRepPress)
						onRepPress();
					_count++;
				}
			} else {
				// handle the two key press;
				if (_allowEvents && _otherKey && _count == 0) {
					if (onTwoPress) {
						onTwoPress(this, _otherKey);
						// this is the only callback we do
						_allowEvents = false;
					}
				}
			}
		}
		break;
	case keyToOff:
		// ignore the key until debounce time expired
		if (millis() >= _nextValidRead) {
			if (!keyState) {
				// when off advance to the next state
				_previousState = keyOff;
				if (_allowEvents && onShortPress && _count == 0)
					// if key was released within the long press time callback
						onShortPress();
				// clean up if active key
				if (_activeKey == this) {
					_count = 0;
					_activeKey = nullptr;
					_otherKey = nullptr;
				}
			} else
				// otherwise it was a glitch
				_previousState = keyOn;
		}
		break;
	}
}

/*
 * Checks if the key is in the on stage
 */
bool SimpleKeyHandler::isPressed() {
	return _previousState == keyOn;
}

//--------------------------------RgbLcdKeyShield----------------------------

/*
 * WTF? DB4 is connected to GPB4, DB5 to GPB3, DB6 to GPB2 and DB7 to GPB1
 * Fasted way translating this is a 15 byte lookup table. Additionally:
 * GPB7 (RS (register select)) is set as most traffic is to the data register
 * GPB6 (R/W) is set low as this is a write operation
 * GPB5 (E) is set high
 *
 * The table is defined as static in the header file so that it is
 * compiled only once when more instances of this class are created.
 */
#ifdef __AVR__
	const uint8_t RgbLcdKeyShieldI2C::_nibbleToPin[16] PROGMEM = {
#else
	const uint8_t RgbLcdKeyShieldI2C::_nibbleToPin[16] = {
#endif // __AVR__
			B10100000,	// 0000
			B10110000,	// 0001
			B10101000,	// 0010
			B10111000,	// 0011
			B10100100,	// 0100
			B10110100,	// 0101
			B10101100,	// 0110
			B10111100,	// 0111
			B10100010,	// 1000
			B10110010,	// 1001
			B10101010,	// 1010
			B10111010,	// 1011
			B10100110,	// 1100
			B10110110,	// 1101
			B10101110,	// 1110
			B10111110	// 1111
			};

RgbLcdKeyShieldI2C::RgbLcdKeyShieldI2C() {
	_shadowGPIOA = B11000000; // set bit 6 (red led) and 7 (green led) high
	_shadowGPIOB = B00100001; // set bit 0 (blue led) and 5 (lcd enable) high
	_shadowDisplayControl = displayControl | displayOnFlag; // set on, no cursor and no blinking
	_shadowEntryModeSet = entryModeSet | left2RightFlag; // left to right, no shift
}

/*
 * initialize the MCP23017 and the LCD
 */
void RgbLcdKeyShieldI2C::begin(void) {
	// give the lcd some time to get ready
	delay(100);
	/*
	 * Set the MCP23017 in 8 bit mode , sequential addressing
	 * disabled and slew rate disabled by writing to
	 * register 0x0b.
	 * As this register is not present in 16 bit mode
	 * we can safely write to it after a hot reset
	 * of the controlling device as in this case the
	 * MCP23017 is already in 8 bit mode which is possible
	 * as the hardware reset of the device is not used.
	 */
	I2c.write(I2Caddr, IOCON, B10101000);
	// set bit 6 (red led) and 7 (green led) high
	I2c.write(I2Caddr, GPIOA, _shadowGPIOA);
	// make bit 7 and 6 outputs
	I2c.write(I2Caddr, IODIRA, B00111111);
	// enable pull-ups on input pins
	I2c.write(I2Caddr, GPPUA, B00111111);
	// set bit 0 (blue led) and 5 (lcd enable) high
	I2c.write(I2Caddr, GPIOB, _shadowGPIOB);
	// set all to output
	I2c.write(I2Caddr, IODIRB, B00000000);
	// invert the 5 bits connected to the keys so that key pressed is high now
	I2c.write(I2Caddr, IPOLA, B00011111);

	/* Initialize the lcd display
	 * For an explanation what is going on see the Wikipedia
	 * Hitachi HD44780 LCD controller entry
	 */

	I2c.start();
	I2c.sendAddress(SLA_W(I2Caddr));
	I2c.sendByte(GPIOB);
	_lcdWrite4(B0011, true);
	delay(5);
	_lcdWrite4(B0011, true);
	_lcdWrite4(B0011, true);
	// should be in 8 bit mode now so set to 4 bit mode
	_lcdWrite4(B0010, true);
	// set 2 lines and 5x8 dots
	_lcdWrite8(functionSet | lineMode2Flag, true);
	// set on, no cursor and no blinking
	_lcdWrite8(_shadowDisplayControl, true);
	// left to right, no shift
	_lcdWrite8(_shadowEntryModeSet, true);
	I2c.stop();

	// Clear entire display
	clear();
	// Return a shifted display to its original position
	home();
}

/*
 * Clear the display and set the cursor in the upper left corner,
 * set left to right (undocumented :( )
 * takes about two milliseconds.
 */
void RgbLcdKeyShieldI2C::clear() {
	_lcdTransmit(clearDisplay, true);
	// Synchronize left2RightFlag;
	_shadowEntryModeSet = entryModeSet | left2RightFlag; // left to right, no shift
	delay(2);
}

/*
 * Set the cursor in the upper left corner,
 * takes about two milliseconds.
 */
void RgbLcdKeyShieldI2C::home() {
	_lcdTransmit(returnHome, true);
	delay(2);
}

/*
 * Sets the position of the cursor at which subsequent characters
 * will appear.
 */
void RgbLcdKeyShieldI2C::setCursor(uint8_t col, uint8_t row) {
	_lcdTransmit(setDdRamAdr | (col + row * 0x40), true);
}

/*
 * Sets the color of the backlight of the display.
 */
void RgbLcdKeyShieldI2C::setColor(colors color) {
	bitWrite(_shadowGPIOA, 6, !(color & clRed));
	bitWrite(_shadowGPIOA, 7, !(color & clGreen));
	bitWrite(_shadowGPIOB, 0, !(color & clBlue));
	I2c.write(I2Caddr, GPIOA, _shadowGPIOA);
	I2c.write(I2Caddr, GPIOB, _shadowGPIOB);
}

/*
 * turn the display pixels on
 */
void RgbLcdKeyShieldI2C::display() {
	_shadowDisplayControl |= displayOnFlag;
	_lcdTransmit(_shadowDisplayControl, true);
}

/*
 * turn the display pixels off
 */
void RgbLcdKeyShieldI2C::noDisplay() {
	_shadowDisplayControl &= ~displayOnFlag;
	_lcdTransmit(_shadowDisplayControl, true);
}

/*
 * Enables the blinking of the selected character
 */
void RgbLcdKeyShieldI2C::blink() {
	_shadowDisplayControl |= blinkOnFlag;
	_lcdTransmit(_shadowDisplayControl, true);
}

/*
 * Disables the blinking of the selected character
 */
void RgbLcdKeyShieldI2C::noBlink() {
	_shadowDisplayControl &= ~blinkOnFlag;
	_lcdTransmit(_shadowDisplayControl, true);
}

/*
 * Enables the cursor
 */
void RgbLcdKeyShieldI2C::cursor() {
	_shadowDisplayControl |= cursorOnFlag;
	_lcdTransmit(_shadowDisplayControl, true);
}

/*
 * Disables the cursor
 */
void RgbLcdKeyShieldI2C::noCursor() {
	_shadowDisplayControl &= ~cursorOnFlag;
	_lcdTransmit(_shadowDisplayControl, true);
}

/*
 * Scrolls the display to the right
 */
void RgbLcdKeyShieldI2C::scrollDisplayRight() {
	_lcdTransmit(curOrDispShift | displayShiftFlag | shiftRightFlag, true);
}

/*
 * Scrolls the display to the left
 */
void RgbLcdKeyShieldI2C::scrollDisplayLeft() {
	_lcdTransmit(curOrDispShift | displayShiftFlag, true);
}

/*
 * All subsequent characters written to the display will go
 * from left to right.
 */
void RgbLcdKeyShieldI2C::leftToRight() {
	_shadowEntryModeSet |= left2RightFlag;
	_lcdTransmit(_shadowEntryModeSet, true);
}

/*
 * All subsequent characters written to the display will go
 * from right to left.
 */
void RgbLcdKeyShieldI2C::rightToLeft() {
	_shadowEntryModeSet &= ~left2RightFlag;
	_lcdTransmit(_shadowEntryModeSet, true);
}

/*
 * Moves the cursor to the right
 */
void RgbLcdKeyShieldI2C::moveCursorRight() {
	_lcdTransmit(curOrDispShift | shiftRightFlag, true);
}

/*
 * Moves the cursor to the left
 */
void RgbLcdKeyShieldI2C::moveCursorLeft() {
	_lcdTransmit(curOrDispShift, true);
}

/*
 * Turns the automatic scrolling of the display on.
 * New characters will appear at the same location and
 * the content of the display will scroll right or left
 * depending of the write direction.
 */

void RgbLcdKeyShieldI2C::autoscroll() {
	_shadowEntryModeSet |= autoShiftFlag;
	_lcdTransmit(_shadowEntryModeSet, true);
}

/*
 * Turns off automatic scrolling of the display.
 */
void RgbLcdKeyShieldI2C::noAutoscroll() {
	_shadowEntryModeSet &= ~autoShiftFlag;
	_lcdTransmit(_shadowEntryModeSet, true);
}

/*
 * Loads a special character
 * The cursor position is lost after this call
 */
void RgbLcdKeyShieldI2C::createChar(uint8_t location, const uint8_t *charmap) {
	location &= 0x7;   // we only have 8 memory locations 0-7
	_lcdTransmit(setCgRamAdr | location << 3, true);
	write(charmap, 8);
	_lcdTransmit(setDdRamAdr, true);   // cursor position is lost
}

#ifdef __AVR__
/*
 * Loads a special character from program memory
 * The cursor position is lost after this call
 */
void RgbLcdKeyShieldI2C::createCharP(uint8_t location, const uint8_t *charmap) {
	location &= 0x7;   // we only have 8 memory locations 0-7
	_lcdTransmit(setCgRamAdr | location << 3, true);
	writeP(charmap, 8);
	_lcdTransmit(setDdRamAdr, true);   // cursor position is lost
}

/*
 * Writes a string in program memory to the display
 */
size_t RgbLcdKeyShieldI2C::printP(const char str[]) {
	size_t n = 0;
	char c = pgm_read_byte(&str[n]);
	I2c.start();
	I2c.sendAddress(SLA_W(I2Caddr));
	I2c.sendByte(GPIOB);
	while (c) {
		_lcdWrite8(c, false);
		c = pgm_read_byte(&str[++n]);
	};
	I2c.stop();
	return n;
}

/*
 * does the same as write(const uint8_t* buffer, size_t size)
 * but from program memory instead
 */
size_t RgbLcdKeyShieldI2C::writeP(const uint8_t* buffer, size_t size) {
	size_t n = 0;
	I2c.start();
	I2c.sendAddress(SLA_W(I2Caddr));
	I2c.sendByte(GPIOB);
	while (n < size) {
		_lcdWrite8(pgm_read_byte(&buffer[n++]), false);
	};
	I2c.stop();
	return n;
}
#endif // __AVR__

/*
 * Writes a character to the screen
 */
size_t RgbLcdKeyShieldI2C::write(uint8_t c) {
	_lcdTransmit(c, false);
	return 1;
}

/*
 * Reads a character from the screen
 */
uint8_t RgbLcdKeyShieldI2C::read() {
	uint8_t value;
	_prepareRead(false);
	value =  _lcdRead8();
	_cleanupRead();
	return value;
}

/*
 * Reads multiple characters from the screen into a buffer
 */
size_t RgbLcdKeyShieldI2C::read(uint8_t* buffer, size_t size) {
	size_t n = 0;
	_prepareRead(false);
	while (n < size) {
		buffer[n++] = _lcdRead8();
	}
	_cleanupRead();
	return n;
}

/*
 * Read the cursor position
 */
uint8_t RgbLcdKeyShieldI2C::getCursor() {
	uint8_t value;
	_prepareRead(true);
	value = _lcdRead8();
	_cleanupRead();
	return value;
}

/*
 * Overrides the standard implementation
 */
size_t RgbLcdKeyShieldI2C::write(const uint8_t* buffer, size_t size) {
	size_t n = 0;
	I2c.start();
	I2c.sendAddress(SLA_W(I2Caddr));
	I2c.sendByte(GPIOB);
	while (n < size) {
		_lcdWrite8(buffer[n++], false);
	}
	I2c.stop();
	return n;
}

/*
 * Read the keys. To be placed in the main loop.
 */
void RgbLcdKeyShieldI2C::readKeys() {
	uint8_t keyState;
	I2c.read(I2Caddr,GPIOA, 1);
	keyState = I2c.receive();
	keyLeft.read(keyState & B0010000);
	keyUp.read(keyState & B0001000);
	keyDown.read(keyState & B00000100);
	keyRight.read(keyState & B00000010);
	keySelect.read(keyState & B00000001);
}

/*
 * Clear all the callback pointers
 */
void RgbLcdKeyShieldI2C::clearKeys() {
	keyLeft.clear();
	keyUp.clear();
	keyDown.clear();
	keyRight.clear();
	keySelect.clear();
}

// Private declarations--------------------------------------------

/*
 * Helper function to write a nibble to the display
 */
void RgbLcdKeyShieldI2C::_lcdWrite4(uint8_t value, bool lcdInstruction) {
	// clear the lcd bits of shadowB
	_shadowGPIOB &= B00000001;
	// Translate the least nibble only
#ifdef __AVR__
	_shadowGPIOB |= pgm_read_byte(&_nibbleToPin[value & B00001111]);
#else
	_shadowGPIOB |= _nibbleToPin[value & B00001111];
#endif // __AVR__
	// if the instruction register is addressed clear bit 7
	if (lcdInstruction)
		_shadowGPIOB &= B01111111;
	// send the data
	I2c.sendByte(_shadowGPIOB);
	// Toggle the enable bit
	_shadowGPIOB ^= B00100000;
	// and send again
	I2c.sendByte(_shadowGPIOB);
}

/*
 * Helper function to write a byte to the display
 */
inline void RgbLcdKeyShieldI2C::_lcdWrite8(uint8_t value, bool lcdInstruction) {
	_lcdWrite4(value >> 4, lcdInstruction);
	_lcdWrite4(value, lcdInstruction);
}

/*
 * Helper function to transmit a byte to the display
 */
void RgbLcdKeyShieldI2C::_lcdTransmit(uint8_t value, bool lcdInstruction) {
	I2c.start();
	I2c.sendAddress(SLA_W(I2Caddr));
	I2c.sendByte(GPIOB);
	_lcdWrite8(value, lcdInstruction);
	I2c.stop();
}

/*
 * Helper function to prepare for a read
 */
void RgbLcdKeyShieldI2C::_prepareRead(bool lcdInstruction) {
	// set lcd data pins of GPIOB as input
	I2c.write(I2Caddr, IODIRB, B00011110);
	I2c.start();
	I2c.sendAddress(SLA_W(I2Caddr));
	I2c.sendByte(GPIOB);
	// clear the lcd bits of shadowB
	_shadowGPIOB &= B00000001;
	if (lcdInstruction)	// set R/W high
		_shadowGPIOB |= B01000000;
	else // set RS, and R/W high
		_shadowGPIOB |= B11000000;
	I2c.sendByte(_shadowGPIOB);
}

/*
 * Helper function to read a nibble from the display
 */
uint8_t RgbLcdKeyShieldI2C::_lcdRead4() {
	uint8_t value = 0;
	uint8_t temp;
	// set enable high
	_shadowGPIOB |= B00100000;
	I2c.sendByte(_shadowGPIOB);
	I2c.stop();
	I2c.read(I2Caddr, GPIOB, 1);
	temp = I2c.receive();
	// clear enable
	_shadowGPIOB &= B11000001;
	I2c.start();
	I2c.sendAddress(SLA_W(I2Caddr));
	I2c.sendByte(GPIOB);
	I2c.sendByte(_shadowGPIOB);
	// translate pin to nibble
	bitWrite(value, 0, bitRead(temp, 4));
	bitWrite(value, 1, bitRead(temp, 3));
	bitWrite(value, 2, bitRead(temp, 2));
	bitWrite(value, 3, bitRead(temp, 1));
	return value;
}

/*
 * Helper function to read a byte from the display
 */
inline uint8_t RgbLcdKeyShieldI2C::_lcdRead8() {
	return (_lcdRead4() << 4) + _lcdRead4();
}

/*
 * Helper function to cleanup after read
 */
inline void RgbLcdKeyShieldI2C::_cleanupRead() {
	I2c.stop();
	// set all pins back as output
	I2c.write(I2Caddr, IODIRB, B00000000);
}
