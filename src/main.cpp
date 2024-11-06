#include <stdlib.h>
#include <time.h>
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

bool display[32][64];
// A program counter, often called just “programCounter”, which points at the
// current instruction in memory
//
// The first CHIP-8 interpreter (on the COSMAC VIP computer) was also located in
// RAM, from address 000 to 1FF. It would expect a CHIP-8 program to be loaded
// into memory after it, starting at address 200 (512 in decimal). Although
// modern interpreters are not in the same memory space, you should do the same
// to be able to run the old programs; you can just leave the initial space
// empty, except for the font.
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
const uint8_t FONT_SET[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


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
	BeginDrawing();
	ClearBackground(BLACK);
	
	for (int y = 0; y < DISPLAY_HEIGHT; y++) {
		for (int x = 0; x < DISPLAY_WIDTH; x++) {
			if (display[y][x]) {
				DrawRectangle(x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, WHITE);
			}
		}
	}

	EndDrawing();
}


void clearScreen() {
	for (int y = 0; y < DISPLAY_HEIGHT; y++) {
		for (int x = 0; x < DISPLAY_WIDTH; x++) {
			display[y][x] = false;
		}
	}
}


// Fetch the instruction from memory at the current programCounter (program
// counter)
const uint16_t fetch() {
	uint8_t d1 = *memory[programCounter];
	programCounter++;
	uint8_t d2 = *memory[programCounter];
	programCounter++;
	uint16_t wd = (d2 << 8) | d1; //bitshift 8 and bitwise OR 
	return wd;
}


const uint8_t readRegister(uint8_t name) {
	switch (name) {
		case 0x0: {
			return V0; 
			break;
		} 
		case 0x1: {
			return V1; 
			break;
		} 
		case 0x2: {
			return V2; 
			break;
		} 
		case 0x3: {
			return V3; 
			break;
		} 
		case 0x4: {
			return V4; 
			break;
		} 
		case 0x5: {
			return V5; 
			break;
		} 
		case 0x6: {
			return V6; 
			break;
		} 
		case 0x7: {
			return V7; 
			break;
		} 
		case 0x8: {
			return V8; 
			break;
		} 
		case 0x9: {
			return V9; 
			break;
		} 
		case 0xA: {
			return VA; 
			break;
		} 
		case 0xB: {
			return VB; 
			break;
		} 
		case 0xC: {
			return VC; 
			break;
		} 
		case 0xD: {
			return VD; 
			break;
		} 
		case 0xE: {
			return VE; 
			break;
		} 
		case 0xF: {
			return VF; 
			break;
		} 
		default:
			//WARN: should not be reached
			return -1;
			break;
	}
}


void overwriteRegister(uint8_t name, uint8_t newValue) {
	switch (name) {
		case 0x0: {
			V0 = newValue;
			return; 
		} 
		case 0x1: {
			V1 = newValue;
			return; 
		} 
		case 0x2: {
			V2 = newValue;
			return; 
		} 
		case 0x3: {
			V3 = newValue;
			return; 
		} 
		case 0x4: {
			V4 = newValue;
			return; 
		} 
		case 0x5: {
			V5 = newValue;
			return; 
		} 
		case 0x6: {
			V6 = newValue;
			return; 
		} 
		case 0x7: {
			V7 = newValue;
			return; 
		} 
		case 0x8: {
			V8 = newValue;
			return; 
		} 
		case 0x9: {
			V9 = newValue;
			return; 
		} 
		case 0xA: {
			VA = newValue;
			return; 
		} 
		case 0xB: {
			VB = newValue;
			return; 
		} 
		case 0xC: {
			VC = newValue;
			return; 
		} 
		case 0xD: {
			VD = newValue;
			return; 
		} 
		case 0xE: {
			VE = newValue;
			return; 
		} 
		case 0xF: {
			VF = newValue;
			return; 
		} 
		default:
			//WARN: should not be reached
			break;
	}
}


// Decode the instruction to find out what the emulator should do
void decode(const uint16_t& instruction) {
	// X: The second nibble. Used to look up one of the 16 registers (VX) from V0 through VF.
	// Y: The third nibble. Also used to look up one of the 16 registers (VY) from V0 through VF.
	// N: The fourth nibble. A 4-bit number.
	// NN: The second byte (third and fourth nibbles). An 8-bit immediate number.
	// NNN: The second, third and fourth nibbles. A 12-bit immediate memory address.
	
	uint8_t firstNibble 	= instruction & 0xF000;
	uint8_t	secondNibble	= instruction & 0x0F00; 
	uint8_t	thirdNibble		= instruction & 0x00F0;
	uint8_t	fourthNibble	= instruction & 0x000F;
	
	switch (firstNibble) {
		case 0x0:
			switch (instruction) {
				case 0x00E0: { 
					clearScreen();
					break;
				}; 
				case 0x00EE: {
					//Returning from subroutine
					programCounter = stack.top();
					stack.pop();
					break;
				};
			}
			break;
		case 0x1: { 
			// 0x1NNN Jump to adress NNN 
			programCounter = instruction & 0x0FFF;
			break;
		} 
		case 0x2: {
			// 0x2NNN Call subroutine at NNN
			stack.push(programCounter);
			programCounter = instruction & 0x0FFF;
			break;
		}
		case 0x3: {
			// 3XNN skip one instruction if VX is equal to NN
			if (readRegister(secondNibble) == (instruction & 0x00FF)) {
				programCounter += 2;
			}
			break;
		}
		case 0x4: {
			// 4XNN skip one instruction if VX is NOT equal to NN
			if (readRegister(secondNibble) != (instruction & 0x00FF)) {
				programCounter += 2;
			}
			break;
		} 
		case 0x5: {
			// 5XY0 skip one instruction if VX and VY are equal
			if (readRegister(secondNibble) == readRegister(thirdNibble)) {
				programCounter += 2;
			}
			break;
		} 
		case 0x6: {
			// 6XNN Simply set VX to the value NN
			overwriteRegister(secondNibble, instruction & 0x00FF);
			break;
		}
		case 0x7: {
			// 7XNN Add NN to VX
			uint8_t newSum = readRegister(secondNibble) + (instruction & 0x00FF);
			overwriteRegister(secondNibble, newSum);
			break;
		} 
		case 0x8: {

			break;
		} 
		case 0x9: {
			// 9XY0 skip one instruction if VX and VY are NOT equal
			if (readRegister(secondNibble) != readRegister(thirdNibble)) {
				programCounter += 2;
			}
			break;
		}
		case 0xA: {
			// ANNN sets register I NNN
			register_I = instruction & 0x0FFF;
			break;
		} 
		case 0xB: {
			// BNNN Jump with offset
			// jump to the address V0 + NNN  
			// WARN: Ambiguous instruction!
			programCounter = readRegister(0x0) + (instruction & 0x0FFF);
			break;
		} 
		case 0xC: {
			// CXNN
			// generate a random number, binary AND it with the value NN, and puts the result in VX
			uint8_t rand8bit = rand()%256;
			overwriteRegister(secondNibble, instruction & 0x00FF);
			break;
		} 
		case 0xD: {
			// DXYN Display
			// Draw an N pixels tall sprite from the memory location that
			// the I index register is holding to the screen, at the
			// horizontal X coordinate in VX and the Y coordinate in VY
      
			// uint8_t X = readRegister(secondNibble) & 63;
      // uint8_t Y = readRegister(thirdNibble) & 31;
			// VF = 0;
			// for (int i = 0; i < fourthNibble; i++) {
			// 	uint8_t spriteData = *memory[register_I + i];
			// 	for (int j = 0; j < 8; j++) {
			// 		if (spriteData & 0x01){
			// 			if (display[Y][X]) {
			// 				display[Y][X] = false;
			// 				VF = 1;
			// 			} else {
			// 				display[Y][X] = true;
			// 			}
			// 		}	
			// 		spriteData = spriteData >> 1;
			// 	}
			// }


			break;
		} 
		case 0xE: {

			break;
		} 
		case 0xF: {

			break;
		} 
	}
}


int main() {
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CHIP-8 EMULATOR");
	SetTargetFPS(FRAMERATE);
	
	srand(time(NULL)); // set random seed using time
	while(!WindowShouldClose()) {
		decrementTimers();
		updateDisplay();
		
		// Fetch -> Decode -> Execute
		// const uint16_t instruction = fetch();
		// decode(instruction);
		// execute(instruction); // just do in decode-switch

	}

	CloseWindow();
}
