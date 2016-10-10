#pragma once

#include "Arduboy.h"

#define WIDTH_DIV8_DIV8 2

class SmartDisplay
{

private:

	/// A reference to the actual ArduBoy library.
	Arduboy* arduboy;

	/// The screen buffer of the arduboy.
	uint8_t* sBuffer;

	/// An array to store the cell updates.
	uint8_t changesToClear[(HEIGHT / 8)*WIDTH_DIV8_DIV8];

	/// Internal function to retrieve in an screen cell has changed before and has to clear.
	inline bool getChangedToClear(uint8_t page, uint8_t col)
	{
		return changesToClear[page*(WIDTH / 8 / 8) + (col / 8)] & (0x01 << (col % 8));
	}

	/// An array to store the cell updates.
	uint8_t changes[(HEIGHT / 8)*WIDTH_DIV8_DIV8];

	/// Internal function to retrieve in an screen cell has changed.
	inline bool getChanged(uint8_t page, uint8_t col)
	{
		return changes[page*(WIDTH / 8 / 8) + (col / 8)] & (0x01 << (col % 8));
	}

	/// Internal function to mark an screen cell as changed.
	inline void setChanged(uint8_t page, uint8_t col, bool value)
	{
		if (value)
			changes[page*(WIDTH / 8 / 8) + (col / 8)] |= 0x01 << (col % 8);
		else
			changes[page*(WIDTH / 8 / 8) + (col / 8)] &= ~(0x01 << (col % 8));
	}	

	/// Last index of page updated.
	uint8_t the_page;

	/// Internal function that clears a list consecutive cells from the screen.
	void clearCells(uint8_t page, uint8_t col_first, uint8_t col_last);

	/// Internal function that updates a list consecutive cells from the screen.
	void updateCells(uint8_t page, uint8_t col_first, uint8_t col_last);

public:

	/// Constructor.
	SmartDisplay(Arduboy* arduboy2) : arduboy(arduboy2), sBuffer((uint8_t*)arduboy->getBuffer()), the_page(-1), LastUpdates(0) {}

	/// Info about the number of updates in the last frame.
	uint8_t LastUpdates;

	/// The swap buffers function.
	void display();

	/// Clear the entire screen.
	void clear();

	/// Draws a rect.
	void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

	/// Draws a bitmap from program memory to a specific X/Y.
	void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color);

	/// Internal function that draws a bitmap from sram memory.
	void drawBitmapFromSram(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color);

	/// Mark some pixels as dirty.
	void markDirty(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

};