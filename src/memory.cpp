#include "../include/memory.h"


uint8_t memory[4096];
uint16_t programCounter; 
uint16_t register_I;
std::stack<uint16_t> stack; 

// 16 one byte general-purpose variable registers 
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

// FONT DATA
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


void loadRomIntoMemory(char* filename) {
	FILE* filePtr = fopen(filename, "rb");
 	
	fseek(filePtr, 0, SEEK_END);          
  long fileLen = ftell(filePtr);            
  rewind(filePtr);      

	uint8_t* fileBuffer = (uint8_t*) malloc(sizeof(uint8_t) * fileLen);
	
	fread(fileBuffer, sizeof(uint8_t), fileLen, filePtr);
	
	for (int i = 0; i < fileLen; i++) {
		printf("Data: %02X, loaded into memory idx: %d\n", fileBuffer[i], i + 0x200);
		memory[i + 0x200] = fileBuffer[i];
	}
}


const uint16_t fetch() {
	uint8_t byte1 = memory[programCounter];
	uint8_t byte2 = memory[programCounter + 1];
	programCounter += 2;
	uint16_t word = (byte1 << 8) | byte2; //bitshift 8 and bitwise OR 
	return word;
}


void initilizeMemory() {
	for (int i = 0; i < 4096; i++) {
		memory[i] = 0x00;
	}
	V0 = 0; 
	V1 = 0;
	V2 = 0;
	V3 = 0;
	V4 = 0;
	V5 = 0;
	V6 = 0;
	V7 = 0;
	V8 = 0;
	V9 = 0;
	VA = 0;
	VB = 0;
	VC = 0;
	VD = 0;
	VE = 0;
	VF = 0;
	
	register_I = 0;
	programCounter = 0x200;
	stack.push(programCounter);
}


void setPC(uint16_t newValue) {
	programCounter = newValue;
}


void callSubroutine(uint16_t pcAddress) {
	stack.push(programCounter);
	programCounter = pcAddress;
}


void returnFromSubroutine() {
	programCounter = stack.top();
	stack.pop();
}


void skipInstruction() {
	programCounter += 2;
}


void setRegisterI(uint16_t newValue){
	register_I = newValue;
}


uint8_t readMemory(uint16_t idx) {
	return memory[idx];
}


void overwriteMemory(uint16_t idx, uint8_t value) {
	memory[idx] = value;
}


uint16_t readRegisterI() {
	return register_I;
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
			//WARN: should never be reached
			printf("UNKNOWN REGISTER V%02X", name);
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
