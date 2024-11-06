#include <raylib.h>
#include <cstdint>
#include <stack>

const int PIXEL_SIZE = 16;
const int DISPLAY_WIDTH = 64;
const int DISPLAY_HEIGHT = 32;
const int SCREEN_WIDTH = DISPLAY_WIDTH * PIXEL_SIZE;
const int SCREEN_HEIGHT = DISPLAY_HEIGHT * PIXEL_SIZE;
const int FRAMERATE = 60; // ideal for decrementing sound-/delayTimer:s
const int HZ_TIMER = 700 * FRAMERATE; // controll instructions executed per second



// Memory: CHIP-8 has direct access to up to 4 kilobytes of RAM
uint8_t* memory[4096];
// The first CHIP-8 interpreter (on the COSMAC VIP computer) was also located in
// RAM, from address 000 to 1FF. It would expect a CHIP-8 program to be loaded
// into memory after it, starting at address 200 (512 in decimal). Although
// modern interpreters are not in the same memory space, you should do the same
// to be able to run the old programs; you can just leave the initial space
// empty, except for the font.

// Display: 64 x 32 pixels (or 128 x 64 for SUPER-CHIP) monochrome, ie. black or
// white

bool** display[32][64];

// A program counter, often called just “programCounter”, which points at the current
// instruction in memory
uint16_t programCounter;

// One 16-bit index register called “I” which is used to point at locations in
// memory
uint16_t register_I;


// A stack for 16-bit addresses, which is used to call subroutines/functions and
// return from them
std::stack<uint16_t> stack;


// An 8-bit delay timer which is decremented at a rate of 60 Hz (60 times per
// second) until it reaches 0
uint8_t delayTimer;

// An 8-bit sound timer which functions like the delay timer, but which also
// gives off a beeping sound as long as it’s not 0
uint8_t soundTimer;


// 16 8-bit (one byte) general-purpose variable registers numbered 0 through F
// hexadecimal, ie. 0 through 15 in decimal, called V0 through VF

uint8_t V0; 
uint8_t V1;
uint8_t V2;
uint8_t V3;
uint8_t V4;
uint8_t V5;
uint8_t V6;
uint8_t V7;
uint8_t V8;
uint8_t V9;
uint8_t VA;
uint8_t VB;
uint8_t VC;
uint8_t VD;
uint8_t VE;
uint8_t VF;

// FONT
//
// 0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
// 0x20, 0x60, 0x20, 0x20, 0x70, // 1
// 0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
// 0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
// 0x90, 0x90, 0xF0, 0x10, 0x10, // 4
// 0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
// 0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
// 0xF0, 0x10, 0x20, 0x40, 0x40, // 7
// 0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
// 0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
// 0xF0, 0x90, 0xF0, 0x90, 0x90, // A
// 0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
// 0xF0, 0x80, 0x80, 0x80, 0xF0, // C
// 0xE0, 0x90, 0x90, 0x90, 0xE0, // D
// 0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
// 0xF0, 0x80, 0xF0, 0x80, 0x80  // F


// KEYPAD
// 1 	2 	3 	4
// Q 	W 	E 	R
// A 	S 	D 	F
// Z 	X 	C 	V



void decrementTimers() {
	if (delayTimer > 0) {
		delayTimer--;
	}
	if (soundTimer > 0) {
		soundTimer--;
	}
}


void updateDisplay() {
	ClearBackground(BLACK);

	for (int y = 0; y < DISPLAY_HEIGHT; y++)
}


const uint16_t fetch() {
	uint8_t d1 = *memory[programCounter];
	programCounter++;
	uint8_t d2 = *memory[programCounter];
	programCounter++;
	uint16_t wd = (d2 << 8) | d1;
	return wd;
}



int main() {
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CHIP-8 EMULATOR");
	
	SetTargetFPS(FRAMERATE);

	while(!WindowShouldClose()) {
		
		decrementTimers();

		// Fetch the instruction from memory at the current programCounter (program
		// counter)

		// Decode the instruction to find out what the emulator should
		// do

		// Execute the instruction and do what it tells you

		BeginDrawing();
		
		ClearBackground(BLACK);

		EndDrawing();
	}

	CloseWindow();
}
