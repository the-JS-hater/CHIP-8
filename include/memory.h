#ifndef MEMORY
#define MEMORY

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <stack>


const uint16_t fetch();

const uint8_t readRegister(uint8_t);

void overwriteRegister(uint8_t , uint8_t);

void initilizeMemory();

void loadFontsIntoMemory();

void loadRomIntoMemory(char*);

void skipInstruction();

void returnFromSubroutine();

void callSubroutine(uint16_t); 

void setPC(uint16_t);

void setRegisterI(uint16_t);

uint8_t readMemory(uint16_t);

void overwriteMemory(uint16_t, uint8_t);

uint16_t readRegisterI();

#endif
