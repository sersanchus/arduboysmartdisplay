#include "SmartDisplay.h"

void SmartDisplay::clearCells(uint8_t page, uint8_t col_first, uint8_t col_last)
{
	uint8_t first_pixel_col = col_first << 3;
	uint8_t last_pixel_col = ((col_last + 1) << 3) - 1;

	uint8_t* pointer = sBuffer + page*WIDTH + first_pixel_col;

	memset(pointer, 0x00, last_pixel_col - first_pixel_col + 1);
}

void SmartDisplay::updateCells(uint8_t page, uint8_t col_first, uint8_t col_last)
{
	uint8_t first_pixel_col = col_first << 3;
	uint8_t last_pixel_col = ((col_last + 1) << 3) - 1;

	arduboy->LCDCommandMode();

	if (the_page != page)
	{
		SPI.transfer(0x22); // page
		SPI.transfer(page); // start
		SPI.transfer(page); // end  
		the_page = page;
	}

	SPI.transfer(0x21); // columns
	SPI.transfer(first_pixel_col); // start
	SPI.transfer(last_pixel_col); // end

	arduboy->LCDDataMode();

	uint8_t* pointer = sBuffer + page*WIDTH + first_pixel_col;
	for (uint8_t i = first_pixel_col; i <= last_pixel_col; i++)
	{
		arduboy->paint8Pixels(*pointer);
		pointer++;
	}

	LastUpdates += col_last - col_first + 1;
}

void SmartDisplay::display()
{
	LastUpdates = 0;

	int first_col_changed;
	int last_col_changed;
	for (int j = 0; j < (HEIGHT / 8); j++)
	{
		first_col_changed = -1;
		for (int i = 0; i < (WIDTH / 8); i++)
		{
			if (getChanged(j, i) || getChangedToClear(j, i))
			{
				if (first_col_changed == -1)
					first_col_changed = last_col_changed = i;
				else
					last_col_changed++;
			}
			else if (first_col_changed != -1)
			{
				updateCells(j, first_col_changed, last_col_changed);
				first_col_changed = last_col_changed = -1;
			}
		}

		if (first_col_changed != -1)
		{
			updateCells(j, first_col_changed, last_col_changed);
			first_col_changed = last_col_changed = -1;
		}
	}

	arduboy->LCDCommandMode();
	SPI.transfer(0x22); // page
	SPI.transfer(0); // start
	SPI.transfer(7); // end  
	SPI.transfer(0x21); // columns
	SPI.transfer(0); // start
	SPI.transfer(127); // end
	arduboy->LCDDataMode();

	memcpy(changesToClear, changes, (HEIGHT / 8)*WIDTH_DIV8_DIV8);
	memset(changes, 0x00, (HEIGHT / 8)*WIDTH_DIV8_DIV8);
}

void SmartDisplay::clear()
{
	int first_col_changed;
	int last_col_changed;
	for (int j = 0; j < (HEIGHT / 8); j++)
	{
		first_col_changed = -1;
		for (int i = 0; i < (WIDTH / 8); i++)
		{
			if (getChangedToClear(j, i))
			{
				if (first_col_changed == -1)
					first_col_changed = last_col_changed = i;
				else
					last_col_changed++;
			}
			else if (first_col_changed != -1)
			{
				clearCells(j, first_col_changed, last_col_changed);
				first_col_changed = last_col_changed = -1;
			}
		}

		if (first_col_changed != -1)
		{
			clearCells(j, first_col_changed, last_col_changed);
			first_col_changed = last_col_changed = -1;
		}
	}
}

void SmartDisplay::drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
	uint8_t pageFirst = y / 8;
	uint8_t pageLast = (y + h - 1) / 8;
	uint8_t colFirst = x / 8;
	uint8_t colLast = (x + w - 1) / 8;

	for (int i = colFirst; i <= colLast; i++)
	{
		setChanged(pageFirst, i, true);
		setChanged(pageLast, i, true);
	}
	for (int j = pageFirst; j <= pageLast; j++)
	{
		setChanged(j, colFirst, true);
		setChanged(j, colLast, true);
	}

	arduboy->drawRect(x, y, w, h, color);
}

void SmartDisplay::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color)
{
	markDirty(x, y, w, h);
	arduboy->drawBitmap(x, y, bitmap, w, h, color);
}

void SmartDisplay::drawBitmapFromSram(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color)
{
	// no need to dar at all of we're offscreen
	if (x + w < 0 || x > WIDTH - 1 || y + h < 0 || y > HEIGHT - 1)
		return;

	markDirty(x, y, w, h);

	int yOffset = abs(y) % 8;
	int sRow = y / 8;
	if (y < 0) {
		sRow--;
		yOffset = 8 - yOffset;
	}
	int rows = h / 8;
	if (h % 8 != 0) rows++;
	for (int a = 0; a < rows; a++) {
		int bRow = sRow + a;
		if (bRow >(HEIGHT / 8) - 1) break;
		if (bRow > -2) {
			for (int iCol = 0; iCol<w; iCol++) {
				if (iCol + x >(WIDTH - 1)) break;
				if (iCol + x >= 0) {
					if (bRow >= 0) {
						if (color == WHITE) this->sBuffer[(bRow*WIDTH) + x + iCol] |= *(bitmap + (a*w) + iCol) << yOffset;
						else if (color == BLACK) this->sBuffer[(bRow*WIDTH) + x + iCol] &= ~(*(bitmap + (a*w) + iCol) << yOffset);
						else                     this->sBuffer[(bRow*WIDTH) + x + iCol] ^= *(bitmap + (a*w) + iCol) << yOffset;
					}
					if (yOffset && bRow<(HEIGHT / 8) - 1 && bRow > -2) {
						if (color == WHITE) this->sBuffer[((bRow + 1)*WIDTH) + x + iCol] |= *(bitmap + (a*w) + iCol) >> (8 - yOffset);
						else if (color == BLACK) this->sBuffer[((bRow + 1)*WIDTH) + x + iCol] &= ~(*(bitmap + (a*w) + iCol) >> (8 - yOffset));
						else                     this->sBuffer[((bRow + 1)*WIDTH) + x + iCol] ^= *(bitmap + (a*w) + iCol) >> (8 - yOffset);
					}
				}
			}
		}
	}
}

void SmartDisplay::markDirty(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t pageFirst = y / 8;
	uint8_t pageLast = (y + h - 1) / 8;
	uint8_t colFirst = x / 8;
	uint8_t colLast = (x + w - 1) / 8;

	uint8_t jaux = pageFirst*WIDTH_DIV8_DIV8;
	for (uint8_t j = pageFirst; j <= pageLast; j++)
	{
		for (uint8_t i = colFirst; i <= colLast; i++)
		{
			changes[jaux + (i >> 3)] |= 0x01 << (i % 8);
		}
		jaux += WIDTH_DIV8_DIV8;
	}
}