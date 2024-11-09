#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <raylib.h>
#include <math.h>
#include <cstdint>
#include <stack>

const int SAMPLE_RATE = 44100;  // Sound sample rate
const float FREQUENCY = 440.0f; // Frequency of the beep (A4 note)
const float DURATION = 1.0f/60.0f;   // Duration of the beep in seconds

const int PIXEL_SIZE = 16;
const int DISPLAY_WIDTH = 64;
const int DISPLAY_HEIGHT = 32;
const int SCREEN_WIDTH = DISPLAY_WIDTH * PIXEL_SIZE;
const int SCREEN_HEIGHT = DISPLAY_HEIGHT * PIXEL_SIZE;
const int FRAMERATE = 60; // ideal for decrementing sound-/delayTimer:s
const int INSTRUCTIONS_PER_FRAME = 700 / 60; // controll instructions executed per second

/*OPTION FLAGS FOR AMBIGOUS INSTRUCTIONS*/
const bool SHIFT_IN_PLACE_FLAG = true; // shift VX in place in 8XY6 & 8XYE 

uint8_t memory[4096];
uint16_t programCounter; 
bool display[32][64];

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
		memory[i] = FONT_SET[i];
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


void decrementTimers(Sound beep) {
	if (delayTimer > 0) {
		delayTimer--;
	}
	if (soundTimer > 0) {
		PlaySound(beep);
		soundTimer--;
	}
}


void updateDisplay() {
	BeginDrawing();
	ClearBackground(BLACK);
	
	printf("Updating display\n");
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
	printf("Clearing the screen\n");
	for (int y = 0; y < DISPLAY_HEIGHT; y++) {
		for (int x = 0; x < DISPLAY_WIDTH; x++) {
			display[y][x] = false;
		}
	}
}


// Fetch the instruction from memory at the current programCounter (program
// counter)
const uint16_t fetch() {
	// if (programCounter % 2 != 0){
	// 	printf("WARNING: unaligned memory access!\n");
	// }
	uint8_t byte1 = memory[programCounter];
	uint8_t byte2 = memory[programCounter + 1];
	programCounter += 2;
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
			printf("UNKNOWN REGISTER V%02X", name);
			break;
	}
}


// Decode (and Execute) a single instruction
void decode(const uint16_t& instruction) {
	printf("Decoding instruction: %04X\n", instruction);
	uint8_t firstNibble  = (instruction & 0xF000) >> 12;
	uint8_t secondNibble = (instruction & 0x0F00) >> 8;
	uint8_t thirdNibble  = (instruction & 0x00F0) >> 4;
	uint8_t fourthNibble = instruction & 0x000F;

	switch (firstNibble) {
		case 0x0: {
			switch (instruction) {
				case 0x00E0: {
					printf("Clear screen\n");
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
		}
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
			printf("comparing VX: %d and NN: %04X\n", readRegister(secondNibble), (instruction & 0x00FF));
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
			switch (fourthNibble) {
				case 0x0: {
					// 8XY0 VX set to VY
					overwriteRegister(secondNibble, readRegister(thirdNibble));
					break;
				}
				case 0x1: {
					// 8XY1 VX |= VY (binary OR compound assignment)
					uint8_t bitwiseOR = readRegister(secondNibble) | readRegister(thirdNibble);
					overwriteRegister(secondNibble, bitwiseOR);
					break;
				}
				case 0x2: {
					// 8XY2 VX &= VY (binary AND compound assignment)
					uint8_t bitwiseAND = readRegister(secondNibble) & readRegister(thirdNibble);
					overwriteRegister(secondNibble, bitwiseAND);
					break;
				}
				case 0x3: {
					// 8XY3: Logical XOR
					// 8XY3 VX ^= VY (binary XOR compound assignment)
					uint8_t bitwiseXOR = readRegister(secondNibble) ^ readRegister(thirdNibble);
					overwriteRegister(secondNibble, bitwiseXOR);
					break;
				}
				case 0x4: {
					// 8XY4 VX += VY
					uint8_t sum = readRegister(secondNibble) + readRegister(thirdNibble);
					overwriteRegister(secondNibble, sum);
					break;
				}
				case 0x5: {
					// 8XY5 VX = VX - VY
					uint8_t vx = readRegister(secondNibble);
					uint8_t vy = readRegister(thirdNibble);
					overwriteRegister(secondNibble, vx - vy);
					VF = vx > vy ? 1 : 0;
					break;
				}
				case 0x6: {
					//WARN: Ambiguous instruction!
					
					// (optional) Set VX to the value of VY
					// Shift the value of VX one bit to the right
					// Set VF to 1 if the bit that was shifted out was 1, or 0 if it was 0
					if (!SHIFT_IN_PLACE_FLAG) {
						overwriteRegister(secondNibble, readRegister(thirdNibble));
					}

					const uint8_t msb_VX = (readRegister(secondNibble) >> 7) & 1;
					VF = (msb_VX == 1) ? 1 : 0;
					
					overwriteRegister(secondNibble, readRegister(secondNibble) << 1);
					break;
        }
				case 0x7: {
					// 8XY7 VX = VY - VX
					uint8_t vx = readRegister(secondNibble);
					uint8_t vy = readRegister(thirdNibble);
					overwriteRegister(secondNibble, vy - vx);
					VF = vy > vx ? 1 : 0;
					break;
				}
				case 0xE: {
					//WARN: Ambiguous instruction!
					
					// Optional, or configurable) Set VX to the value of VY
					// Shift the value of VX one bit to the left
					// Set VF to 1 if the bit that was shifted out was 1, or 0 if it was 0
					if (!SHIFT_IN_PLACE_FLAG) {
						overwriteRegister(secondNibble, readRegister(thirdNibble));
					}

					const uint8_t lsb_VX = (readRegister(secondNibble) & 1);
					VF = (lsb_VX == 1) ? 1 : 0;
					
					overwriteRegister(secondNibble, readRegister(secondNibble) << 1);
					break;
				}
			}
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
			uint8_t rand8bit = rand() % 256;
			uint8_t nn = instruction & 0x00FF;
			printf("random num: %d\n", rand8bit);
			overwriteRegister(secondNibble, rand8bit & nn);
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
			  uint8_t spriteData = memory[register_I + i];  
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
						programCounter += 2;
					}
					break;
				}
				case 0x1: {
					// EXA1 skips if the key corresponding to the value in VX is not pressed
					uint8_t key_idx = readRegister(secondNibble);
					if (!checkKeypress(key_idx)) {
						programCounter += 2;
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
						programCounter -= 2; 
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
					memory[register_I] = (num - num % 100) / 100;
					memory[register_I + 1] = (num % 100 - num % 10) / 10;
					memory[register_I + 2] = num % 10;
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
		default: {
			printf("UNKNOWN INSTRUCTION: %04X\n", instruction);
		}
	}
}


int main(int argc, char** argv) {
		
	//keep raylib from cluttering the terminal
	SetTraceLogLevel(LOG_NONE);
  // TODO: make a function for initializing/setting all standard values
  // for registers, PC, and such...
	for (int i = 0; i < 4096; i++) {
		memory[i] = 0x00;
	}

	//TODO: make a separate function to load ROM
	char* fileName = argv[1];
	FILE* filePtr = fopen(argv[1], "rb");
 	
	fseek(filePtr, 0, SEEK_END);          
  long fileLen = ftell(filePtr);            
  rewind(filePtr);      

	uint8_t* fileBuffer = (uint8_t*) malloc(sizeof(uint8_t) * fileLen);
	
	fread(fileBuffer, sizeof(uint8_t), fileLen, filePtr);
	
	for (int i = 0; i < fileLen; i++) {
		printf("Data: %02X, loaded into memory idx: %d\n", fileBuffer[i], i + 0x200);
		memory[i + 0x200] = fileBuffer[i];
	}
	loadFontsIntoMemory();

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CHIP-8 EMULATOR");
	SetTargetFPS(FRAMERATE);
	srand(time(NULL)); // set random seed using time
	

	//TODO: move all this audio stuff into separate function 
	InitAudioDevice();  // Initialize audio device and context
	// Generate sine wave sample data for a beep
    unsigned int sampleCount = (unsigned int)(SAMPLE_RATE * DURATION);
    short *data = (short *)malloc(sampleCount * sizeof(short));
    for (int i = 0; i < sampleCount; i++) {
        data[i] = (short)(32767 * sinf(2 * PI * FREQUENCY * i / SAMPLE_RATE));
    }

  Wave wave = {
		sampleCount,
		SAMPLE_RATE,
		16,
		1,
    data
  };

  Sound beepSound = LoadSoundFromWave(wave);


	programCounter = 0x200;
	stack.push(programCounter);

	while(!WindowShouldClose()) {
		decrementTimers(beepSound);

		for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++){
			printf("PC: %d", programCounter);
			printf(" Stack size: %d\n", (int)stack.size());
			const uint16_t instruction = fetch();
			printf("Instruction fetched: %04X\n", instruction);
			decode(instruction);
		}

		updateDisplay();
	}

	CloseWindow();
}
