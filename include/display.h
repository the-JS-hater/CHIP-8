#ifndef DISPLAY
#define DISPLAY


#include <raylib.h>
#include <stdio.h>
#include <cstdint>


const int PIXEL_SIZE = 16;
const int DISPLAY_WIDTH = 64;
const int DISPLAY_HEIGHT = 32;
const int SCREEN_WIDTH = DISPLAY_WIDTH * PIXEL_SIZE;
const int SCREEN_HEIGHT = DISPLAY_HEIGHT * PIXEL_SIZE;


void updateDisplay();
void clearScreen();
void XORontoDisplay(int, int);
bool readPixel(int, int);

#endif
