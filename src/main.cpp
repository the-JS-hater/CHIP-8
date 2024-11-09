#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <raylib.h>
#include <math.h>
#include <cstdint>
#include "../include/memory.h"

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

bool display[32][64];

uint8_t delayTimer = 0;

uint8_t soundTimer = 0;


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
					returnFromSubroutine();
					break;
				};
			}
			break;
		}
		case 0x1: { 
			// 0x1NNN Jump to adress NNN
			setPC(instruction & 0x0FFF);
			break;
		} 
		case 0x2: {
			// 0x2NNN Call subroutine at NNN
			callSubroutine(instruction & 0x0FFF);
			break;
		}
		case 0x3: {
			// 3XNN skip one instruction if VX is equal to NN
			if (readRegister(secondNibble) == (instruction & 0x00FF)) {
				skipInstruction();
			}
			break;
		}
		case 0x4: {
			// 4XNN skip one instruction if VX is NOT equal to NN
			printf("comparing VX: %d and NN: %04X\n", readRegister(secondNibble), (instruction & 0x00FF));
			if (readRegister(secondNibble) != (instruction & 0x00FF)) {
				skipInstruction();
			}
			break;
		} 
		case 0x5: {
			// 5XY0 skip one instruction if VX and VY are equal
			if (readRegister(secondNibble) == readRegister(thirdNibble)) {
				skipInstruction();
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
					uint8_t vf = vx > vy ? 1 : 0;
					overwriteRegister(0xF, vf);
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
					uint8_t vf = (msb_VX == 1) ? 1 : 0;
					overwriteRegister(0xF, vf);
					overwriteRegister(secondNibble, readRegister(secondNibble) << 1);
					break;
        }
				case 0x7: {
					// 8XY7 VX = VY - VX
					uint8_t vx = readRegister(secondNibble);
					uint8_t vy = readRegister(thirdNibble);
					overwriteRegister(secondNibble, vy - vx);
					uint8_t vf = vy > vx ? 1 : 0;
					overwriteRegister(0xF, vf);
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
					uint8_t vf = (lsb_VX == 1) ? 1 : 0;
					overwriteRegister(0xF, vf);
					overwriteRegister(secondNibble, readRegister(secondNibble) << 1);
					break;
				}
			}
			break;
		} 
		case 0x9: {
			// 9XY0 skip one instruction if VX and VY are NOT equal
			if (readRegister(secondNibble) != readRegister(thirdNibble)) {
				skipInstruction();
			}
			break;
		}
		case 0xA: {
			// ANNN sets register I NNN
			setRegisterI(instruction & 0x0FFF);
			break;
		} 
		case 0xB: {
			// BNNN Jump with offset
			// jump to the address V0 + NNN  
			// WARN: Ambiguous instruction!
			setPC(readRegister(0x0) + (instruction & 0x0FFF));
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
			overwriteRegister(0xF, 0);

			for (int i = 0; i < fourthNibble; i++) {
			  uint8_t spriteData = readMemory(readRegisterI() + i);
			  for (int j = 0; j < 8; j++) {
			    // Get each bit from the most significant to least significant
					uint8_t pixel = (spriteData >> (7 - j)) & 0x01;
	
					// Calculate actual screen coordinates with wrapping
					int posX = (x + j) % DISPLAY_WIDTH;
					int posY = (y + i) % DISPLAY_HEIGHT;
	
					// XOR the pixel onto the display and update VF if there's a collision
					if (pixel) {
						if (display[posY][posX]) {
							overwriteRegister(0xF, 1);
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
						skipInstruction();
					}
					break;
				}
				case 0x1: {
					// EXA1 skips if the key corresponding to the value in VX is not pressed
					uint8_t key_idx = readRegister(secondNibble);
					if (!checkKeypress(key_idx)) {
						skipInstruction();
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
					
					uint16_t newValue = readRegister(secondNibble) + readRegisterI(); 
					if (newValue > 0x0FFF) {
						overwriteRegister(0xF, 1);
					} 

					setRegisterI(newValue);
					break;
				}
				case 0x0A: {
					//Make it loop indefinitley untill keypress
					if (GetKeyPressed() == 0) {
						skipInstruction();
					}
					break;
				}
				case 0x29: {
					// FX29 register I set to the address of the hexadecimal character in VX
					setRegisterI(readRegister(secondNibble) * 5);
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
					uint16_t I_idx = readRegisterI();
					
					overwriteMemory(I_idx, (num - num % 100) / 100);
					overwriteMemory(I_idx + 1, (num % 100 - num % 10) / 10);
					overwriteMemory(I_idx + 2, num % 10);
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
  
	//set all registers and memory addresses to 0
	//set PC to 0x200 and push it onto stack
	initilizeMemory();
	//load font data into first 80 addresses
	loadFontsIntoMemory();

	char* fileName = argv[1];
  	// load rom data into memeory, starting at adress 0x200
  loadRomIntoMemory(fileName);


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

	while(!WindowShouldClose()) {
		decrementTimers(beepSound);

		for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++){
			const uint16_t instruction = fetch();
			decode(instruction);
		}

		updateDisplay();
	}

	CloseWindow();
}
