#include "../include/display.h"


bool display[32][64];


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


void XORontoDisplay(int posX, int posY) {
	display[posY][posX] ^= true;
}


bool readPixel(int posX, int posY) {
	return display[posY][posX];
}
