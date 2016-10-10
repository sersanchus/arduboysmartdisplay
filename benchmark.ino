#include "Arduboy.h"
#include "SmartDisplay.h"

/// Global constants.
#define PARTICLE_SIZE 16
#define PARTICLE_RADIUS 4
#define BORDER 1
#define X_MIN (PARTICLE_RADIUS+BORDER)
#define Y_MIN (PARTICLE_RADIUS+BORDER)
#define X_MAX (WIDTH-1-PARTICLE_RADIUS-BORDER)
#define Y_MAX (HEIGHT-1-PARTICLE_RADIUS-BORDER)
#define ORBIT_RADIUS 16
#define X_MIN_PARTICLE (ORBIT_RADIUS+PARTICLE_RADIUS+PARTICLE_RADIUS+BORDER)
#define X_MAX_PARTICLE (WIDTH-1-ORBIT_RADIUS-PARTICLE_RADIUS-PARTICLE_RADIUS-BORDER)
#define MAX_PARTICLES 10

/// Char buffer for text output
char text[8];

/// Number of particles to be rendered
uint8_t numParticles = 5; // MAX_PARTICLES;

/// Seeds and rotatetion counters
uint8_t seed1[MAX_PARTICLES];
uint8_t seed2[MAX_PARTICLES];
uint8_t seed3[MAX_PARTICLES];
uint8_t seed4[MAX_PARTICLES];

/// Position of every particle
uint8_t x[MAX_PARTICLES];
uint8_t y[MAX_PARTICLES];

/// Is there any button pressed?
bool anyButtonPressed = false;

/// Smart Display switch
bool smartDisplayUse = true;

/// Pause mode
bool pauseMode = false;

/// Ardoboy instance
Arduboy arduboy;

/// Smart Display instance
SmartDisplay* smartDisplay;

/*/// Bitmap of our particle in 8x8

// , , , , , , , ,
// , , , , , , , ,
// , ,#,#,#,#, , ,
// , ,#, , ,#, , ,
// , ,#, , ,#, , ,
// , ,#,#,#,#, , ,
// , , , , , , , ,
// , , , , , , ,

PROGMEM const uint8_t particleBitmap[] = { 0, 0, 60, 36, 36, 60, 0, 0 };*/

/// Bitmap of our particle in 16x16

// , , , , , , , , , , , , , , , ,
// , , , , , , , , , , , , , , , ,
// , , , , , , , , , , , , , , , ,
// , , ,#,#,#,#,#,#,#,#,#,#, , , ,
// , , ,#, , , , , , , , ,#, , , ,
// , , ,#, , , , , , , , ,#, , , ,
// , , ,#, , , , , , , , ,#, , , ,
// , , ,#, , , , , , , , ,#, , , ,

// , , ,#, , , , , , , , ,#, , , ,
// , , ,#, , , , , , , , ,#, , , ,
// , , ,#, , , , , , , , ,#, , , ,
// , , ,#, , , , , , , , ,#, , , ,
// , , ,#,#,#,#,#,#,#,#,#,#, , , ,
// , , , , , , , , , , , , , , , ,
// , , , , , , , , , , , , , , , ,
// , , , , , , , , , , , , , , , 

PROGMEM const uint8_t particleBitmap[] =
{
	0, 0, 0, 248, 8, 8, 8, 8, 8, 8, 8, 8, 248, 0, 0, 0,
	0, 0, 0, 31, 16, 16, 16, 16, 16, 16, 16, 16, 31, 0, 0, 0
};

/// An array to store the rotated particle
uint8_t particleBitmapRotated[PARTICLE_SIZE*(PARTICLE_SIZE>>3)];

/// 2*PI.
#define M_2_PI 6.283185307179586476925286766559

/// Num of diferent values for angle
#define ANGLE_VALUES 256

/// Cosinus table
int8_t cos_table[ANGLE_VALUES];

/// Fast cosinus function
inline int_fast8_t fastCos(uint_fast8_t radians)
{
	return cos_table[radians];
}

/// Fast sinus function
inline int_fast8_t fastSin(uint_fast8_t radians)
{
	return cos_table[(ANGLE_VALUES + (ANGLE_VALUES >> 2) - radians) % ANGLE_VALUES];
}

/// Arduboy setup
void setup()
{
	// setup the table of cosinus
	float angle;
	short aux;
	for (int i = 0; i < ANGLE_VALUES; i++)
	{
		// angles from 0 [0] to (ANGLE_VALUES-1) [almost 2pi]
		angle = i * M_2_PI / ANGLE_VALUES;
		aux = (short)(cos(angle)*128.0 + 0.5);
		cos_table[i] = (char)min(127, max(-127, aux));
	}

	arduboy.beginNoLogo();
	//arduboy.begin();	

	// framerate
	//arduboy.setFrameRate(24);

	// seeds and angles of every particle
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		seed1[i] = (rand()*ANGLE_VALUES) / (RAND_MAX + 1);
		seed2[i] = (rand()*ANGLE_VALUES) / (RAND_MAX + 1);
		seed3[i] = 5 + ((rand() * 5) / (RAND_MAX + 1));
		seed4[i] = 1 + ((rand() * 5) / (RAND_MAX + 1));
	}

	// our smart display instance
	smartDisplay = new SmartDisplay(&arduboy);
}

/// Rotate the particle bitmap an angle between 0 and ANGLE_VALUES
void rotateParticle(uint_fast8_t angle)
{
	bool pixelIsWhite;
	uint_fast8_t packedBits;
	int_fast16_t rotatedX;
	int_fast16_t rotatedY;
	uint_fast8_t particle_x;
	uint_fast8_t particle_y;
	int_fast8_t cosine = fastCos(angle);
	int_fast8_t sine = fastSin(angle);

	for (uint_fast8_t page = 0; page < PARTICLE_SIZE >> 3; page++)
	{
		// for every column then compound from different rows
		for (uint_fast8_t i = 0; i < PARTICLE_SIZE; i++)
		{
			packedBits = 0x00;
			particle_x = i;
			for (uint_fast8_t j = 0; j < 8; j++)
			{
				particle_y = (page << 3) + j;
				rotatedX = (int_fast16_t)(((PARTICLE_SIZE >> 1) + ((((particle_x - (PARTICLE_SIZE >> 1))*cosine) - ((particle_y - (PARTICLE_SIZE >> 1))*sine)) / 127)));
				rotatedY = (int_fast16_t)(((PARTICLE_SIZE >> 1) + ((((particle_x - (PARTICLE_SIZE >> 1))*sine) + ((particle_y - (PARTICLE_SIZE >> 1))*cosine)) / 127)));
				if (rotatedX >= 0 && rotatedX < PARTICLE_SIZE && rotatedY >= 0 && rotatedY < PARTICLE_SIZE)
				{
					pixelIsWhite = pgm_read_byte(particleBitmap + ((rotatedY >> 3)*PARTICLE_SIZE) + rotatedX) & (0x01 << (rotatedY%8));
					packedBits |= pixelIsWhite << j;
				}
			}
			particleBitmapRotated[(page*PARTICLE_SIZE) + i] = packedBits;
		}
	}
}

/// Draw all particles.
void draw_particles()
{
	for (int i = 0; i < numParticles; i++)
	{
		uint8_t x_axis = X_MIN_PARTICLE + ((X_MAX_PARTICLE - X_MIN_PARTICLE)*(fastCos(seed1[i])+127) / 255);

		x[i] = x_axis + ((fastCos(seed2[i]) * ORBIT_RADIUS) / 127);
		y[i] = (HEIGHT >> 1) + ((fastSin(seed2[i]) * ORBIT_RADIUS) / 127);
		
		rotateParticle(seed2[i]);
		smartDisplay->drawBitmapFromSram(x[i]- (PARTICLE_SIZE>>1), y[i] - (PARTICLE_SIZE >> 1), particleBitmapRotated, PARTICLE_SIZE, PARTICLE_SIZE, 1);

		if (seed1[i] + seed4[i] < ANGLE_VALUES)
			seed1[i] += seed4[i];
		else
			seed1[i] = seed1[i] + seed4[i] - ANGLE_VALUES;

		if (seed2[i] + seed3[i] < ANGLE_VALUES)
			seed2[i] += seed3[i];
		else
			seed2[i] = seed2[i] + seed3[i] - ANGLE_VALUES;
	}
}

/// Draw info table.
void draw_info()
{
	sprintf(text, "cpu:%d", arduboy.cpuLoad());
	arduboy.setCursor(82, 5);
	arduboy.print(text);
	sprintf(text, " ms:%d", arduboy.lastFrameDurationMs);
	arduboy.setCursor(82, 15);
	arduboy.print(text);
	sprintf(text, "num:%d", numParticles);
	arduboy.setCursor(82, 25);
	arduboy.print(text);
	sprintf(text, "upd:%d", smartDisplay->LastUpdates);
	arduboy.setCursor(82, 35);
	arduboy.print(text);

	smartDisplay->markDirty(82, 1, 42, 45);
}

/// Main loop.
void loop()
{
	// pause render until it's time for the next frame
	if (!(arduboy.nextFrame()))
		return;

	// button up?
	if (anyButtonPressed && !arduboy.pressed(A_BUTTON) && !arduboy.pressed(B_BUTTON) && !arduboy.pressed(UP_BUTTON) && !arduboy.pressed(DOWN_BUTTON))
		anyButtonPressed = false;

	// has to pause?
	if (arduboy.pressed(B_BUTTON) && !anyButtonPressed)
	{
		pauseMode = !pauseMode;
		anyButtonPressed = true;
	}

	// paused mode
	if (pauseMode)
	{
		smartDisplay->clear();
		smartDisplay->drawRect(0, 0, WIDTH, HEIGHT, 1);
		draw_info();
	}
	else
	{		
		if (arduboy.pressed(A_BUTTON))
		{
			if (!anyButtonPressed)
			{
				smartDisplayUse = !smartDisplayUse;
				anyButtonPressed = true;
			}
		}
		else if (arduboy.pressed(UP_BUTTON))
		{
			if (!anyButtonPressed)
			{
				if (numParticles < MAX_PARTICLES)
					numParticles++;
				anyButtonPressed = true;
			}
		}
		else if (arduboy.pressed(DOWN_BUTTON))
		{
			if (!anyButtonPressed)
			{
				if (numParticles > 1)
					numParticles--;
				anyButtonPressed = true;
			}
		}

		if (smartDisplayUse)
		{
			smartDisplay->clear();
		}
		else
		{			
			arduboy.clear();
			arduboy.drawRect(0, 0, WIDTH, HEIGHT, 1);
		}

		smartDisplay->drawRect(0, 0, WIDTH, HEIGHT, 1);
		draw_particles();
		draw_info();		
	}

	// then we finaly we tell the arduboy to display what we just wrote to the display
	if (smartDisplayUse)
	{
		smartDisplay->display();
	}
	else
	{
		arduboy.display();
		smartDisplay->LastUpdates = 128;
	}
}