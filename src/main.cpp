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
uint16_t programCounter = 0x200; 

// One 16-bit index register called “I” which is used to point at locations in
// memory
uint16_t register_I = 0;


// A stack for 16-bit addresses, which is used to call subroutines/functions and
// return from them
std::stack<uint16_t> stack;


// An 8-bit delay timer which is decremented at a rate of 60 Hz (60 times per
// second) until it reaches 0
uint8_t delayTimer = 0;


// An 8-bit sound timer which functions like the delay timer, but which also
// gives off a beeping sound as long as it’s not 0
uint8_t soundTimer = 0;


// 16 8-bit (one byte) general-purpose variable registers 
uint8_t V0 = 0; 
uint8_t V1 = 0;
uint8_t V2 = 0;
uint8_t V3 = 0;
uint8_t V4 = 0;
uint8_t V5 = 0;
uint8_t V6 = 0;
uint8_t V7 = 0;
uint8_t V8 = 0;
uint8_t V9 = 0;
uint8_t VA = 0;
uint8_t VB = 0;
uint8_t VC = 0;
uint8_t VD = 0;
uint8_t VE = 0;
uint8_t VF = 0;


// FONT
uint8_t FONT_SET[80] = {
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


void loadFontsIntoMemory() {
	for (int i = 0; i < 80; i++) {
		memory[i] = &FONT_SET[i];
	}
}


// KEYPAD
uint8_t keypad[16] = {
	// 1 	2 	3 	4
	// Q 	W 	E 	R
	// A 	S 	D 	F
	// Z 	X 	C 	V
	0x0, 0x1, 0x2, 0x3,
	0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xA, 0xB,
	0xC, 0xD, 0xE, 0xF,
}; 


bool checkKeypress(uint8_t key_idx) {
	switch (key_idx) {
		case 0x0: {
			return IsKeyPressed(KEY_ONE);
		}
		case 0x1: {
			return IsKeyPressed(KEY_TWO);
		}
		case 0x2: {
			return IsKeyPressed(KEY_THREE);
		}
		case 0x3: {
			return IsKeyPressed(KEY_FOUR);
		}
		case 0x4: {
			return IsKeyPressed(KEY_Q);
		}
		case 0x5: {
			return IsKeyPressed(KEY_W);
		}
		case 0x6: {
			return IsKeyPressed(KEY_E);
		}
		case 0x7: {
			return IsKeyPressed(KEY_R);
		}
		case 0x8: {
			return IsKeyPressed(KEY_A);
		}
		case 0x9: {
			return IsKeyPressed(KEY_S);
		}
		case 0xA: {
			return IsKeyPressed(KEY_D);
		}
		case 0xB: {
			return IsKeyPressed(KEY_F);
		}
		case 0xC: {
			return IsKeyPressed(KEY_Z);
		}
		case 0xD: {
			return IsKeyPressed(KEY_X);
		}
		case 0xE: {
			return IsKeyPressed(KEY_C);
		}
		case 0xF: {
			return IsKeyPressed(KEY_V);
		}
		default: //WARN: should be unreachable
			return false;
	}
}


void decrementTimers() {
	if (delayTimer > 0) {
		delayTimer--;
	}
	if (soundTimer > 0) {
		//TODO: play sound
		
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
	programCounter++;
	uint8_t byte1 = *memory[programCounter * 2];
	uint8_t byte2 = *memory[programCounter * 2 + 1];
	uint16_t word = (byte1 << 8) | byte2; //bitshift 8 and bitwise OR 
	return word;
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


// Decode (and Execute) a single instruction
void decode(const uint16_t& instruction) {
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
			//TODO: 
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

			uint8_t x = readRegister(secondNibble) & 63;
			uint8_t y = readRegister(thirdNibble) & 31;
			VF = 0;
			
			for (int i = 0; i < fourthNibble; i++) {
			  uint8_t spriteData = *memory[register_I + i];  
			  for (int j = 0; j < 8; j++) {
			    // Get each bit from the most significant to least significant
					uint8_t pixel = (spriteData >> (7 - j)) & 0x01;
	
					// Calculate actual screen coordinates with wrapping
					int posX = (x + j) % DISPLAY_WIDTH;
					int posY = (y + i) % DISPLAY_HEIGHT;
	
					// XOR the pixel onto the display and update VF if there's a collision
					if (pixel) {
						if (display[posY][posX]) {
							VF = 1;
			      }
			      display[posY][posX] ^= true;
			    }
			  }
			}
			break;
		} 
		case 0xE: {
			switch (fourthNibble) {
				// Since the keypad is hexadecimal, the valid values here are keys 0–F
				case 0xE: {
					// EX9E will skip one instruction if the key corresponding to the value in VX is pressed
					uint8_t key_idx = readRegister(secondNibble);
					if (checkKeypress(key_idx)) {
						programCounter++;
					}
					break;
				}
				case 0x1: {
					// EXA1 skips if the key corresponding to the value in VX is not pressed
					uint8_t key_idx = readRegister(secondNibble);
					if (!checkKeypress(key_idx)) {
						programCounter++;
					}
					break;
				}
			}
			break;
		} 
		case 0xF: {
			switch(instruction & 0x00FF){
				case 0x07: {
					// FX07 sets VX to current value delay timer
					overwriteRegister(secondNibble, delayTimer);
					break;
				}
				case 0x15: {
					// FX15 set delay timer to VX
					delayTimer = readRegister(secondNibble);
					break;
				}
				case 0x18: {
					// FX18 set sound timer to VX
					soundTimer = readRegister(secondNibble);
					break;
				}
				case 0x1E: {
					// FX1E I will get the value in VX added to it.
					// WARN: ambigiuos instruction
					// if I "overflows" (> 0FFF), set VF = 1
					
					uint16_t newValue = readRegister(secondNibble) + register_I;
					if (newValue > 0x0FFF) VF = 1;
					register_I = newValue;
					
					break;
				}
				case 0x0A: {
					//Make it loop indefinitley untill keypress
					if (GetKeyPressed() == 0) {
						programCounter--; 
					}
					break;
				}
				case 0x29: {
					// FX29 register I set to the address of the hexadecimal character in VX
					register_I = readRegister(secondNibble) * 5;	
					break;
				}
				case 0x33: {
 					// FX33
 					// It takes the number in VX (which is
 					// one byte, so it can be any number
 					// from 0 to 255) and converts it to
 					// three decimal digits, storing these
 					// digits in memory at the address in
 					// the index register I. 
					
					uint8_t num = readRegister(secondNibble);
					*memory[register_I] = (num - num % 100) / 100;
					*memory[register_I + 1] = (num % 100 - num % 10) / 10;
					*memory[register_I + 2] = num % 10;
          break;
				}
				case 0x55: {
					//TODO: 
					//WARN: Ambiguous instruction!
					
					break;
				}
				case 0x65: {
					//TODO: 
					//WARN: Ambiguous instruction!
					
					break;
				}
			}
			break;
		} 
	}
}


int main() {

	loadFontsIntoMemory();

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CHIP-8 EMULATOR");
	SetTargetFPS(FRAMERATE);
	
	srand(time(NULL)); // set random seed using time
	while(!WindowShouldClose()) {
		decrementTimers();
		updateDisplay();
		
		// Fetch -> Decode -> Execute
		// const uint16_t instruction = fetch();
		// decode(instruction);
		
		// should execute hz_timer amount of instructions
		// increment PC here presumably?
	}

	CloseWindow();
}
